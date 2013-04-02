/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdint.h>
#include <stdexcept>
#include <memory>
#include <string>
#include <vector>
#include <iterator>
#include "utf8.h"
#include "core/engine.hpp"
#include "native.h"

using namespace RHVoice;

namespace
{
  class java_exception: public std::runtime_error
  {
  public:
    java_exception():
      std::runtime_error("Java exception")
    {
    }
  };

  class internal_exception: public std::runtime_error
  {
  public:
    explicit internal_exception(const std::string& msg):
      std::runtime_error(msg)
    {
    }
  };

  class no_voices: public internal_exception
  {
  public:
    no_voices():
      internal_exception("No voices found")
    {
    }
  };

  void inline check(JNIEnv* env)
  {
    if(env->ExceptionCheck())
      throw java_exception();
  }

  template<typename O>
  O inline check(JNIEnv* env,const O& obj)
  {
    if(obj!=0)
      return obj;
    if(env->ExceptionCheck())
      throw java_exception();
    return obj;
  }

  template<typename T>
  inline T* get_native_field(JNIEnv* env,jobject obj,jfieldID field_id)
  {
    long lval=env->GetLongField(obj,field_id);
    check(env);
    return reinterpret_cast<T*>(static_cast<intptr_t>(lval));
  }

  template<typename T>
  inline void set_native_field(JNIEnv* env,jobject obj,jfieldID field_id,T* ptr)
  {
    env->SetLongField(obj,field_id,reinterpret_cast<intptr_t>(ptr));
    check(env);
  }

  inline void clear_native_field(JNIEnv* env,jobject obj,jfieldID field_id)
  {
    void* ptr=0;
    env->SetLongField(obj,field_id,reinterpret_cast<intptr_t>(ptr));
    check(env);
  }

  jstring string_to_jstring(JNIEnv* env,const std::string& cppstr)
  {
    std::vector<jchar> jchars;
    utf8::utf8to16(cppstr.begin(),cppstr.end(),std::back_inserter(jchars));
    return check(env,env->NewString(&jchars[0],jchars.size()));
  }

  class jstring_handle
  {
  public:
    jstring_handle(JNIEnv* env_,jstring jstr_):
    env(env_),
    jstr(jstr_),
      jlength(env->GetStringLength(jstr)),
      jchars(check(env,env->GetStringChars(jstr,0)))
    {
    }

    ~jstring_handle()
    {
      env->ReleaseStringChars(jstr,jchars);
    }

    const jchar* str() const
    {
      return jchars;
    }

    jsize length() const
    {
      return jlength;
    }

  private:
    JNIEnv* env;
    jstring jstr;
    jsize jlength;
    const jchar* jchars;

    jstring_handle(const jstring_handle&);
    jstring_handle& operator=(const jstring_handle&);
  };

  std::string inline jstring_to_string(JNIEnv* env,jstring jstr)
  {
    jstring_handle handle(env,jstr);
    std::string result;
    utf8::utf16to8(handle.str(),handle.str()+handle.length(),std::back_inserter(result));
    return result;
  }

  jclass RHVoiceException_class;
  jfieldID data_field;
  jclass VoiceInfo_class;
  jmethodID VoiceInfo_constructor;
  jmethodID VoiceInfo_setName_method;

  class Data
  {
  public:
    Data()
    {
    }

    smart_ptr<engine> engine_ptr;

  private:
    Data(const Data&);
    Data& operator=(const Data&);
  };
}

#define TRY try {

#define CATCH1(env) } \
    catch(const java_exception&) {} \
    catch(const std::exception& e) {env->ThrowNew(RHVoiceException_class,e.what());} \
  return;

#define CATCH2(env,retval) } \
    catch(const java_exception&) {} \
    catch(const std::exception& e) {env->ThrowNew(RHVoiceException_class,e.what());} \
  return retval;

JNIEXPORT void JNICALL Java_com_github_olga_1yakovleva_rhvoice_TTSEngine_onClassInit
  (JNIEnv *env, jclass TTSEngine_class)
{
  TRY
    RHVoiceException_class=check(env,env->FindClass("com/github/olga_yakovleva/rhvoice/RHVoiceException"));
  data_field=check(env,env->GetFieldID(TTSEngine_class,"data","J"));
  VoiceInfo_class=check(env,env->FindClass("com/github/olga_yakovleva/rhvoice/VoiceInfo"));
  VoiceInfo_constructor=check(env,env->GetMethodID(VoiceInfo_class,"<init>","()V"));
  VoiceInfo_setName_method=check(env,env->GetMethodID(VoiceInfo_class,"setName","(Ljava/lang/String;)V"));
  CATCH1(env)
}

JNIEXPORT void JNICALL Java_com_github_olga_1yakovleva_rhvoice_TTSEngine_onInit
  (JNIEnv *env, jobject obj, jstring data_path, jstring config_path)
{
  TRY
  clear_native_field(env,obj,data_field);
  std::auto_ptr<Data> data(new Data);
  data->engine_ptr=engine::create();
  if(data->engine_ptr->get_voices().empty())
    throw no_voices();
  set_native_field(env,obj,data_field,data.get());
  data.release();
  CATCH1(env)
}

JNIEXPORT void JNICALL Java_com_github_olga_1yakovleva_rhvoice_TTSEngine_onShutdown
  (JNIEnv *env, jobject obj)
{
  TRY
  delete get_native_field<Data>(env,obj,data_field);
  clear_native_field(env,obj,data_field);
  CATCH1(env)
}

JNIEXPORT jobjectArray JNICALL Java_com_github_olga_1yakovleva_rhvoice_TTSEngine_doGetVoices
  (JNIEnv *env, jobject obj)
{
  TRY
    Data* data=get_native_field<Data>(env,obj,data_field);
  const voice_list& voices=data->engine_ptr->get_voices();
  std::size_t count=std::distance(voices.begin(),voices.end());
  jobject default_value=check(env,env->NewObject(VoiceInfo_class,VoiceInfo_constructor));
  jobjectArray result=check(env,env->NewObjectArray(count,VoiceInfo_class,default_value));
  std::size_t i=0;
  for(voice_list::const_iterator it=voices.begin();it!=voices.end();++it,++i)
    {
      jobject jvoice=check(env,env->NewObject(VoiceInfo_class,VoiceInfo_constructor));
      const std::string& name=it->get_name();
      jstring jname=string_to_jstring(env,name);
      env->CallVoidMethod(jvoice,VoiceInfo_setName_method,jname);
      check(env);
      env->SetObjectArrayElement(result,i,jvoice);
      check(env);
    }
  return result;
  CATCH2(env,0)
}
