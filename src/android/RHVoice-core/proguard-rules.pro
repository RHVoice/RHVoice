-dontobfuscate
-keep class com.github.olga_yakovleva.rhvoice.RHVoiceException
{
<init>(...);
}
-keep class com.github.olga_yakovleva.rhvoice.LanguageInfo
{
<init>(...);
<methods>;
}
-keep class com.github.olga_yakovleva.rhvoice.VoiceInfo
{
<init>(...);
<methods>;
}
-keep class com.github.olga_yakovleva.rhvoice.SynthesisParameters
{
<methods>;
}
-keep class com.github.olga_yakovleva.rhvoice.LogLevel
{
<init>(...);
<methods>;
<fields>;
}
-keep class com.github.olga_yakovleva.rhvoice.** implements com.github.olga_yakovleva.rhvoice.Logger
{
<methods>;
}
-keep class com.github.olga_yakovleva.rhvoice.** implements com.github.olga_yakovleva.rhvoice.TTSClient
{
<methods>;
}
-keep class com.github.olga_yakovleva.rhvoice.TTSEngine
{
<methods>;
<fields>;
}
-keep class com.github.olga_yakovleva.rhvoice.android.TtsResource
{
<methods>;
<fields>;
}
-keep class com.github.olga_yakovleva.rhvoice.android.LanguageResource
{
<methods>;
<fields>;
}
-keep class com.github.olga_yakovleva.rhvoice.android.VoiceResource
{
<init>(...);
<fields>;
}
-keep class com.github.olga_yakovleva.rhvoice.android.PackageDirectory
{
<methods>;
<fields>;
}
-keep class com.github.olga_yakovleva.rhvoice.android.VoiceAccent
{
<methods>;
<fields>;
}
-dontwarn com.android.org.conscrypt.SSLParametersImpl
-dontwarn org.apache.harmony.xnet.provider.jsse.SSLParametersImpl
