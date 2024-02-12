#include <stdio.h>
#include "libShareAudio.h"
#include "../../src/ShareAudio.h" 
JNIEXPORT jint JNICALL Java_libShareAudio_SA_1TestDLL (JNIEnv * env, jclass class){
  return SA_TestDLL();
}

JNIEXPORT void JNICALL Java_libShareAudio_SA_1ListAllAudioDevices(JNIEnv *env, jclass class, jlong ptr){
  return SA_ListAllAudioDevices((void *)ptr);
}

JNIEXPORT jstring JNICALL Java_libShareAudio_SA_1Version(JNIEnv *env, jclass class){
  return (*env)->NewStringUTF(env,SA_Version());
}

JNIEXPORT void JNICALL Java_libShareAudio_SA_1SetLogCONSOLE(JNIEnv *env, jclass class, jint isDegub){
  return SA_SetLogCONSOLE(isDegub);
}

JNIEXPORT jlong JNICALL Java_libShareAudio_SA_1Setup(JNIEnv *env, jclass class, jint device, jstring host, jint mode, 
                                                    jint port, jint testMode, jint channels, jfloat volMod, jint waveSize, jdouble sampleRate){
  const char *cString = (*env)->GetStringUTFChars(env, host, NULL);
  return (jlong)SA_Setup(device,cString,mode,port,testMode,channels,volMod,waveSize,sampleRate);
}

JNIEXPORT void JNICALL Java_libShareAudio_SA_1Init(JNIEnv *env, jclass class, jlong ptr){
  SA_Init((void *)ptr);
}

JNIEXPORT void JNICALL Java_libShareAudio_SA_1Client(JNIEnv *env, jclass class, jlong ptr){
  SA_Client((void *)ptr);
}

JNIEXPORT void JNICALL Java_libShareAudio_SA_1Close(JNIEnv *env, jclass class, jlong ptr){
  SA_Close((void *)ptr);
}

JNIEXPORT void JNICALL Java_libShareAudio_SA_1SetVolumeModifier(JNIEnv *env, jclass class, jfloat volMod, jlong ptr){
  SA_SetVolumeModifier(volMod,(void *)ptr);
}

JNIEXPORT jstring JNICALL Java_libShareAudio_SA_1GetStats(JNIEnv *env, jclass class, jlong ptr){
  const char * strData = SA_GetStats((void *)ptr);
  return (*env)->NewStringUTF(env,strData);
}

JNIEXPORT jfloat JNICALL Java_libShareAudio_SA_1GetVolumeModifier(JNIEnv *env, jclass class, jlong ptr){
  return SA_GetVolumeModifier((void*)ptr);
}

JNIEXPORT void JNICALL Java_libShareAudio_SA_1InitWavRecord(JNIEnv *env, jclass class, jlong ptr, jstring path){
  const char * pathC = (*env)->GetStringUTFChars(env,path,NULL);
  SA_InitWavRecord((void *)ptr,pathC);
}

JNIEXPORT void JNICALL Java_libShareAudio_SA_1CloseWavRecord(JNIEnv *env, jclass class){
  SA_CloseWavRecord();
}