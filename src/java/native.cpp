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
  template<typename T>
  inline T* get_native_field(JNIEnv* env,jobject obj,jfieldID field_id)
  {
    return reinterpret_cast<T*>(static_cast<intptr_t>(env->GetLongField(obj,field_id)));
  }

  template<typename T>
  inline void set_native_field(JNIEnv* env,jobject obj,jfieldID field_id,T* ptr)
  {
    env->SetLongField(obj,field_id,reinterpret_cast<intptr_t>(ptr));
  }

  inline void clear_native_field(JNIEnv* env,jobject obj,jfieldID field_id)
  {
    void* ptr=0;
    env->SetLongField(obj,field_id,reinterpret_cast<intptr_t>(ptr));
  }

  jstring string_to_jstring(JNIEnv* env,const std::string& cppstr)
  {
    std::vector<jchar> jchars;
    utf8::utf8to16(cppstr.begin(),cppstr.end(),std::back_inserter(jchars));
    return env->NewString(&jchars[0],jchars.size());
  }

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

JNIEXPORT void JNICALL Java_com_github_olga_1yakovleva_rhvoice_TTSEngine_onClassInit
  (JNIEnv *env, jclass TTSEngine_class)
{
  data_field=env->GetFieldID(TTSEngine_class,"data","J");
  VoiceInfo_class=env->FindClass("com/github/olga_yakovleva/rhvoice/VoiceInfo");
  VoiceInfo_constructor=env->GetMethodID(VoiceInfo_class,"<init>","()V");
  VoiceInfo_setName_method=env->GetMethodID(VoiceInfo_class,"setName","(Ljava/lang/String;)V");
}

JNIEXPORT void JNICALL Java_com_github_olga_1yakovleva_rhvoice_TTSEngine_onInit
  (JNIEnv *env, jobject obj, jstring data_path, jstring config_path)
{
  clear_native_field(env,obj,data_field);
  std::auto_ptr<Data> data(new Data);
  data->engine_ptr=engine::create();
  set_native_field(env,obj,data_field,data.get());
  data.release();
}

JNIEXPORT void JNICALL Java_com_github_olga_1yakovleva_rhvoice_TTSEngine_onShutdown
  (JNIEnv *env, jobject obj)
{
  delete get_native_field<Data>(env,obj,data_field);
  clear_native_field(env,obj,data_field);
}

JNIEXPORT jobjectArray JNICALL Java_com_github_olga_1yakovleva_rhvoice_TTSEngine_doGetVoices
  (JNIEnv *env, jobject obj)
{
  Data* data=get_native_field<Data>(env,obj,data_field);
  const voice_list& voices=data->engine_ptr->get_voices();
  std::size_t count=std::distance(voices.begin(),voices.end());
  jobject default_value=env->NewObject(VoiceInfo_class,VoiceInfo_constructor);
  jobjectArray result=env->NewObjectArray(count,VoiceInfo_class,default_value);
  std::size_t i=0;
  for(voice_list::const_iterator it=voices.begin();it!=voices.end();++it,++i)
    {
      jobject jvoice=env->NewObject(VoiceInfo_class,VoiceInfo_constructor);
      const std::string& name=it->get_name();
      jstring jname=string_to_jstring(env,name);
      env->CallVoidMethod(jvoice,VoiceInfo_setName_method,jname);
      env->SetObjectArrayElement(result,i,jvoice);
    }
  return result;
}
