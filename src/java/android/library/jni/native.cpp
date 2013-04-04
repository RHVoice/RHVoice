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

  inline jclass find_class(JNIEnv* env,const char* name)
  {
    jclass local_cls=check(env,env->FindClass(name));
    jclass global_cls=check(env,static_cast<jclass>(env->NewGlobalRef(local_cls)));
    return global_cls;
  }

  inline jmethodID get_method(JNIEnv* env,jclass cls,const char* name,const char* sig)
  {
    return check(env,env->GetMethodID(cls,name,sig));
  }

  inline jmethodID get_default_constructor(JNIEnv* env,jclass cls)
  {
    return get_method(env,cls,"<init>","()V");
  }

  inline jmethodID get_string_setter(JNIEnv* env,jclass cls,const char* name)
  {
    return get_method(env,cls,name,"(Ljava/lang/String;)V");
  }

  inline jfieldID get_field(JNIEnv* env,jclass cls,const char* name,const char* sig)
  {
    return check(env,env->GetFieldID(cls,name,sig));
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

  inline std::string jstring_to_string(JNIEnv* env,jstring jstr)
  {
    jstring_handle handle(env,jstr);
    std::string result;
    utf8::utf16to8(handle.str(),handle.str()+handle.length(),std::back_inserter(result));
    return result;
  }

  inline jobject new_object(JNIEnv* env,jclass cls,jmethodID default_constructor)
  {
    return check(env,env->NewObject(cls,default_constructor));
  }

  inline jobjectArray new_object_array(JNIEnv* env,jsize size,jclass cls,jmethodID default_constructor)
  {
    jobject default_value=new_object(env,cls,default_constructor);
    return check(env,env->NewObjectArray(size,cls,default_value));
  }

  inline void set_object_array_element(JNIEnv* env,jobjectArray array,jsize index,jobject value)
  {
    env->SetObjectArrayElement(array,index,value);
    check(env);
  }

  inline void call_string_setter(JNIEnv* env,jobject obj,jmethodID method_id,const std::string& str)
  {
    jstring jstr=string_to_jstring(env,str);
    env->CallVoidMethod(obj,method_id,jstr);
    check(env);
  }

  jclass RHVoiceException_class;
  jfieldID data_field;
  jclass VoiceInfo_class;
  jmethodID VoiceInfo_constructor;
  jmethodID VoiceInfo_setName_method;
  jmethodID VoiceInfo_setLanguage_method;
  jclass LanguageInfo_class;
  jmethodID LanguageInfo_constructor;
  jmethodID LanguageInfo_setName_method;
  jmethodID LanguageInfo_setAlpha2Code_method;
  jmethodID LanguageInfo_setAlpha3Code_method;

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
    RHVoiceException_class=find_class(env,"com/github/olga_yakovleva/rhvoice/RHVoiceException");
  LanguageInfo_class=find_class(env,"com/github/olga_yakovleva/rhvoice/LanguageInfo");
  LanguageInfo_constructor=get_default_constructor(env,LanguageInfo_class);
  LanguageInfo_setName_method=get_string_setter(env,LanguageInfo_class,"setName");
  LanguageInfo_setAlpha2Code_method=get_string_setter(env,LanguageInfo_class,"setAlpha2Code");
  LanguageInfo_setAlpha3Code_method=get_string_setter(env,LanguageInfo_class,"setAlpha3Code");
  VoiceInfo_class=find_class(env,"com/github/olga_yakovleva/rhvoice/VoiceInfo");
  VoiceInfo_constructor=get_default_constructor(env,VoiceInfo_class);
  VoiceInfo_setName_method=get_string_setter(env,VoiceInfo_class,"setName");
  VoiceInfo_setLanguage_method=get_method(env,VoiceInfo_class,"setLanguage","(Lcom/github/olga_yakovleva/rhvoice/LanguageInfo;)V");
  data_field=get_field(env,TTSEngine_class,"data","J");
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
  jobjectArray result=new_object_array(env,count,VoiceInfo_class,VoiceInfo_constructor);
  std::size_t i=0;
  for(voice_list::const_iterator it=voices.begin();it!=voices.end();++it,++i)
    {
      jobject jvoice=new_object(env,VoiceInfo_class,VoiceInfo_constructor);
      const std::string& name=it->get_name();
      call_string_setter(env,jvoice,VoiceInfo_setName_method,name);
      const language_info& lang=*(it->get_language());
      jobject jlanguage=new_object(env,LanguageInfo_class,LanguageInfo_constructor);
      const std::string& language_name=lang.get_name();
      call_string_setter(env,jlanguage,LanguageInfo_setName_method,language_name);
      const std::string& alpha2_code=lang.get_alpha2_code();
      if(!alpha2_code.empty())
        call_string_setter(env,jlanguage,LanguageInfo_setAlpha2Code_method,alpha2_code);
      const std::string& alpha3_code=lang.get_alpha3_code();
      if(!alpha3_code.empty())
        call_string_setter(env,jlanguage,LanguageInfo_setAlpha3Code_method,alpha3_code);
      env->CallVoidMethod(jvoice,VoiceInfo_setLanguage_method,jlanguage);
      check(env);
      set_object_array_element(env,result,i,jvoice);
      env->DeleteLocalRef(jlanguage);
      env->DeleteLocalRef(jvoice);
    }
  return result;
  CATCH2(env,0)
}
