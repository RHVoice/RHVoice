/* Copyright (C) 2012, 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 2 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include <locale>
#include <memory>
#include <iostream>
#include <map>
#include <queue>
#include <algorithm>
#include <giomm.h>

#include "core/engine.hpp"
#include "core/document.hpp"
#include "core/client.hpp"
#include "common.hpp"

namespace
{
  typedef std::vector<gint16> speech_fragment;
  class session;
  typedef std::shared_ptr<session> session_ptr;
  typedef std::map<Glib::ustring,session_ptr> session_map;
  class task;
  typedef std::shared_ptr<task> task_ptr;

  void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connection,const Glib::ustring& name);
  void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connection,const Glib::ustring& name);
  void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connection,const Glib::ustring& name);
  void on_name_appeared(const Glib::RefPtr<Gio::DBus::Connection>& connection,const Glib::ustring& name,const Glib::ustring& name_owner);
  void on_name_vanished(const Glib::RefPtr<Gio::DBus::Connection>& connection,const Glib::ustring& name);
  void on_method_call(const Glib::RefPtr<Gio::DBus::Connection>& connection,
                      const Glib::ustring& sender,
                      const Glib::ustring& object_path,
                      const Glib::ustring& interface_name,
                      const Glib::ustring& method_name,
                      const Glib::VariantContainerBase& params,
                      const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation);
  session_ptr get_session(const Glib::RefPtr<Gio::DBus::Connection>& connection,const Glib::ustring& name,bool if_exists=true);

  const Glib::ustring introspection_xml(
                                        "<node>"
                                        "<interface name='com.github.OlgaYakovleva.RHVoice'>"
                                        "<method name='SpeakText'>"
                                        "<arg name='text' type='s' direction='in'/>"
                                        "</method>"
                                        "<method name='SetPitch'>"
                                        "<arg name='pitch' type='d' direction='in'/>"
                                        "</method>"
                                        "<method name='GetPitch'>"
                                        "<arg name='pitch' type='d' direction='out'/>"
                                        "</method>"
                                        "<method name='SetRate'>"
                                        "<arg name='rate' type='d' direction='in'/>"
                                        "</method>"
                                        "<method name='GetRate'>"
                                        "<arg name='rate' type='d' direction='out'/>"
                                        "</method>"
                                        "<method name='SetVolume'>"
                                        "<arg name='volume' type='d' direction='in'/>"
                                        "</method>"
                                        "<method name='GetVolume'>"
                                        "<arg name='volume' type='d' direction='out'/>"
                                        "</method>"
                                        "<method name='SetSpeakers'>"
                                        "<arg name='speakers' type='s' direction='in'/>"
                                        "</method>"
                                        "<signal name='SpeechAvailable'>"
                                        "<arg name='samples' type='an' direction='out'/>"
                                        "</signal>"
                                        "<signal name='Finished'/>"
                                        "</interface>"
                                        "</node>");

  Glib::RefPtr<Gio::DBus::NodeInfo> introspection_data;
  const Gio::DBus::InterfaceVTable interface_vtable(sigc::ptr_fun(&on_method_call));
  bool name_acquired=false;
  guint registered_id = 0;
  Glib::RefPtr<Glib::MainLoop> main_loop;
  Glib::ThreadPool thread_pool;
  session_map sessions;
  std::shared_ptr<RHVoice::engine> global_engine_ref;

  struct quit_main_loop
  {
    void operator()() const
    {
      main_loop->quit();
    }
  };

  template <typename T>
  T extract(const Glib::VariantBase& base)
  {
    Glib::Variant<T> derived=Glib::VariantBase::cast_dynamic<Glib::Variant<T> >(base);
    return derived.get();
  }

  template<typename T>
  T extract_child(const Glib::VariantContainerBase& container,gsize index=0)
  {
    Glib::Variant<T> child;
    container.get_child(child,index);
    return child.get();
  }

  template<typename T>
  class result_dispatcher
  {
  public:
    explicit result_dispatcher(const sigc::slot<void,const T&> handler)
    {
      data_sent.connect(sigc::mem_fun(*this,&result_dispatcher::dispatch));
      data_received.connect(handler);
    }

    void operator()(const T& value)
    {
      push(value);
      data_sent();
    }

  private:
    result_dispatcher(const result_dispatcher&);
    result_dispatcher& operator=(const result_dispatcher&);

    void push(const T& value)
    {
      Glib::Threads::Mutex::Lock lock(data_mutex);
      data_queue.push(value);
    }

    T pop()
    {
      Glib::Threads::Mutex::Lock lock(data_mutex);
      T result=data_queue.front();
      data_queue.pop();
      return result;
    }

    void dispatch()
    {
      T value=pop();
      data_received(value);
    }

    Glib::Dispatcher data_sent;
    sigc::signal<void,const T&> data_received;
    Glib::Threads::Mutex data_mutex;
    std::queue<T> data_queue;
  };

  class session
  {
  public:
    session(const Glib::RefPtr<Gio::DBus::Connection>& connection,const Glib::ustring& name);

    result_dispatcher<speech_fragment> speech_available;
    Glib::Dispatcher task_finished;

    bool is_speaking() const
    {
      return speaking;
    }

    void stop();

    bool is_stopping() const
    {
      return g_atomic_int_get(&stopping);
    }

    void close();

    bool is_closing() const
    {
      return closing;
    }

    void speak_text(const Glib::ustring& text);

    void set_pitch(double value)
    {
      pitch=std::max(-1.0,std::min(1.0,value));
    }

    double get_pitch() const
    {
      return pitch;
    }

    void set_rate(double value)
    {
      rate=std::max(-1.0,std::min(1.0,value));
    }

    double get_rate() const
    {
      return rate;
    }

    void set_volume(double value)
    {
      volume=std::max(-1.0,std::min(1.0,value));
    }

    double get_volume() const
    {
      return volume;
    }

    bool set_speakers(const Glib::ustring& spec)
    {
      std::string new_speakers=global_engine_ref->create_voice_profile(spec).get_name();
      if(!new_speakers.empty())
        {
          speakers=new_speakers;
          return true;
        }
      else
        return false;
    }

    std::string get_speakers() const
    {
      return speakers;
    }

      private:
    session(const session&);
    session& operator=(const session&);

    void launch_task(const task_ptr& new_task);

    void emit_signal(const Glib::ustring& signal_name)
    {
      connection->emit_signal(RHVoice::service::object_path,
                              RHVoice::service::interface_name,
                              signal_name,
                              name);
    }

    template<typename T>
    void emit_signal(const Glib::ustring& signal_name,const T& arg)
    {
      Glib::Variant<T> var_arg=Glib::Variant<T>::create(arg);
      Glib::VariantContainerBase var_args=Glib::VariantContainerBase::create_tuple(var_arg);
      connection->emit_signal(RHVoice::service::object_path,
                              RHVoice::service::interface_name,
                              signal_name,
                              name,
                              var_args);
    }

    void on_speech_available(const speech_fragment& samples);
    void on_task_finished();

    void set_stopping(bool value=true)
    {
      g_atomic_int_set(&stopping,value);
    }

    Glib::RefPtr<Gio::DBus::Connection> connection;
    Glib::ustring name;
    bool speaking;
    volatile gint stopping;
    bool closing;
    double pitch,rate,volume;
    std::string speakers;
  };

  class clear_session
  {
  public:

    explicit clear_session(const Glib::ustring& name_):
      name(name_)
    {
    }

    void operator()() const
    {
      sessions.erase(name);
    }

  private:
    Glib::ustring name;
  };

  class task: public RHVoice::client
  {
  public:
    void run();
    bool play_speech(const short* samples,std::size_t count);

  protected:
    task(session& parent_session,const Glib::ustring& text_);

    session& parent;
    std::shared_ptr<RHVoice::engine> local_engine_ref;
    RHVoice::voice_profile speakers;
    Glib::ustring text;

  private:
    virtual std::unique_ptr<RHVoice::document> create_document() const=0;
    double pitch,rate,volume;
  };

  class run_task
  {
  public:
    explicit run_task(const task_ptr& t):
      task_to_run(t)
    {
    }

    void operator()()
    {
      task_to_run->run();
    }

  private:
    task_ptr task_to_run;
  };

  class text_task: public task
  {
  public:
    text_task(session& parent_session,const Glib::ustring& text_):
      task(parent_session,text_)
    {
    }

  private:
    std::unique_ptr<RHVoice::document> create_document() const;
  };

  session::session(const Glib::RefPtr<Gio::DBus::Connection>& connection_,const Glib::ustring& name_):
    speech_available(sigc::mem_fun(*this,&session::on_speech_available)),
    connection(connection_),
    name(name_),
    speaking(false),
    stopping(false),
    closing(false),
    pitch(0),
    rate(0),
    volume(0)
  {
    task_finished.connect(sigc::mem_fun(*this,&session::on_task_finished));
  }

  void session::speak_text(const Glib::ustring& text)
  {
    task_ptr new_task(new text_task(*this,text));
    launch_task(new_task);
  }

  void session::launch_task(const task_ptr& new_task)
  {
    speaking=true;
    thread_pool.push(run_task(new_task));
  }

  void session::stop()
  {
    if(is_speaking())
      set_stopping();
  }

  void session::close()
  {
    stop();
    closing=true;
    if(!is_speaking())
      Glib::signal_idle().connect_once(clear_session(name));
  }

  void session::on_speech_available(const speech_fragment& samples)
  {
    if(is_stopping())
      return;
    emit_signal("SpeechAvailable",samples);
  }

  void session::on_task_finished()
  {
    if(is_closing())
      Glib::signal_idle().connect_once(clear_session(name));
    else
      {
        speaking=false;
            set_stopping(false);
            emit_signal("Finished");
      }
  }

  session_ptr get_session(const Glib::RefPtr<Gio::DBus::Connection>& connection,const Glib::ustring& name,bool if_exists)
  {
    session_map::iterator pos=sessions.find(name);
    if(pos!=sessions.end())
      return pos->second;
    if(if_exists)
      return session_ptr();
    session_ptr new_session(new session(connection,name));
    sessions.insert(session_map::value_type(name,new_session));
    Gio::DBus::watch_name(connection,name,sigc::ptr_fun(&on_name_appeared),sigc::ptr_fun(&on_name_vanished));
    return new_session;
  }

  task::task(session& parent_session,const Glib::ustring& text_):
    parent(parent_session),
    local_engine_ref(global_engine_ref),
    text(text_),
    pitch(parent_session.get_pitch()),
    rate(parent_session.get_rate()),
    volume(parent_session.get_volume())
  {
    speakers=local_engine_ref->create_voice_profile(parent.get_speakers());
  }

  void task::run()
  {
    try
      {
        std::unique_ptr<RHVoice::document> doc=create_document();
        if(!parent.is_stopping())
          {
            doc->speech_settings.absolute.rate=rate;
            doc->speech_settings.absolute.pitch=pitch;
            doc->speech_settings.absolute.volume=volume;
            doc->set_owner(*this);
            doc->synthesize();
          }
      }
    catch(const std::exception& e)
      {
        std::cerr << "Synthesis Error: '" << e.what() << "'" << std::endl;
      }
    parent.task_finished();
  }

  bool task::play_speech(const short* samples,std::size_t count)
  {
    if(parent.is_stopping())
      return false;
    else
      {
        parent.speech_available(speech_fragment(samples,samples+count));
        return true;
      }
  }

  std::unique_ptr<RHVoice::document> text_task::create_document() const
  {
    return RHVoice::document::create_from_plain_text(local_engine_ref,text.begin(),text.end(),RHVoice::content_text,speakers);
  }

  void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connection,const Glib::ustring& name)
  {
    if(!connection)
      return;
    try
      {
        registered_id=connection->register_object(RHVoice::service::object_path,
                                                  introspection_data->lookup_interface(RHVoice::service::interface_name),
                                                    interface_vtable);
      }
    catch(const Glib::Error& ex)
      {
        std::cerr << "Registration failed" << std::endl;
      }
  }

  void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connection,const Glib::ustring& name)
  {
    name_acquired=true;
  }

  void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connection,const Glib::ustring& name)
  {
    if(connection)
      {
        if(!name_acquired)
          std::cerr << "Unable to acquire the name" << std::endl;
        if(registered_id)
          connection->unregister_object(registered_id);
      }
    else
      std::cerr << "Unable to connect to the bus" << std::endl;
    Glib::signal_idle().connect_once(quit_main_loop());
  }

  void on_name_appeared(const Glib::RefPtr<Gio::DBus::Connection>& connection,const Glib::ustring& name,const Glib::ustring& name_owner)
  {
  }

  void on_name_vanished(const Glib::RefPtr<Gio::DBus::Connection>& connection,const Glib::ustring& name)
  {
    session_ptr session_to_close=get_session(connection,name);
    if(session_to_close)
      session_to_close->close();
  }

  void on_method_call(const Glib::RefPtr<Gio::DBus::Connection>& connection,
                      const Glib::ustring& sender,
                      const Glib::ustring& object_path,
                      const Glib::ustring& interface_name,
                      const Glib::ustring& method_name,
                      const Glib::VariantContainerBase& params,
                      const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation)
  {
    session_ptr current_session=get_session(connection,sender,false);
    if(current_session->is_closing())
      {
        Gio::DBus::Error error(Gio::DBus::Error::INVALID_ARGS,"Invalid sender");
        invocation->return_error(error);
      }
    if((current_session->is_speaking())&&
       (method_name=="SpeakText"))
      {
        Gio::DBus::Error error(Gio::DBus::Error::INVALID_ARGS,"Previous request is still being processed");
        invocation->return_error(error);
      }
    Glib::VariantContainerBase result;
    if(method_name=="SpeakText")
      current_session->speak_text(extract_child<Glib::ustring>(params));
    else if(method_name=="SetPitch")
      current_session->set_pitch(extract_child<double>(params));
    else if(method_name=="GetPitch")
      result=Glib::VariantContainerBase::create_tuple(Glib::Variant<double>::create(current_session->get_pitch()));
    else if(method_name=="SetRate")
      current_session->set_rate(extract_child<double>(params));
    else if(method_name=="GetRate")
      result=Glib::VariantContainerBase::create_tuple(Glib::Variant<double>::create(current_session->get_rate()));
    else if(method_name=="SetVolume")
      current_session->set_volume(extract_child<double>(params));
    else if(method_name=="GetVolume")
      result=Glib::VariantContainerBase::create_tuple(Glib::Variant<double>::create(current_session->get_volume()));
    else if(method_name=="SetSpeakers")
      {
        if(!current_session->set_speakers(extract_child<Glib::ustring>(params,0)))
          {
            Gio::DBus::Error error(Gio::DBus::Error::INVALID_ARGS,"No voices found");
            invocation->return_error(error);
          }
      }
    else
      {
        Gio::DBus::Error error(Gio::DBus::Error::UNKNOWN_METHOD,"Method does not exist.");
        invocation->return_error(error);
      }
    invocation->return_value(result);
  }
}

int main()
{
  std::locale::global(std::locale::classic());
  try
    {
      global_engine_ref=RHVoice::engine::create();
    }
  catch(const std::exception& e)
    {
      std::cerr << "Engine initialization error: '" << e.what() << "'" << std::endl;
      return 1;
    }
  Gio::init();
  try
    {
      introspection_data=Gio::DBus::NodeInfo::create_for_xml(introspection_xml);
    }
  catch(const Glib::Error& ex)
    {
      std::cerr << "Unable to create introspection data: " << ex.what() << std::endl;
      return 1;
    }
  const guint id=Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                     RHVoice::service::well_known_name,
                                     sigc::ptr_fun(&on_bus_acquired),
                                     sigc::ptr_fun(&on_name_acquired),
                                     sigc::ptr_fun(&on_name_lost));
  main_loop=Glib::MainLoop::create();
  main_loop->run();
  Gio::DBus::unown_name(id);
  return 0;
}
