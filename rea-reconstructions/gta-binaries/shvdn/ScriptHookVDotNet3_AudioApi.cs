// Reconstructed from REA static evidence of ScriptHookVDotNet3.dll
// SHA-256: f328a36445c1d3e282e70b2791efc0d516ae3009d0d1461bf076893805a01cd9
// Metadata: v4.0.30319 | MVID: 6cc7ddfb-691c-4a68-ad59-29fdced17d39 | TypeDef: 303 | MethodDef: 4099
using System;

namespace GTA
{
    public enum AudioFlags
    {
        ActivateSwitchWheelAudio = 0,
        AllowCutsceneOverScreenFade = 1,
        // ... reconstructed from Constant table
    }

    public static class Audio
    {
        // Decoded signatures from rea inspect-managed-members
        public static void PlayMusic(string musicFile)
        {
            // Invokes RAGE native TRIGGER_MUSIC_EVENT
            throw new NotImplementedException("Native call bridge");
        }

        public static void StopMusic(string musicFile)
        {
            // Invokes RAGE native CANCEL_MUSIC_EVENT
            throw new NotImplementedException("Native call bridge");
        }

        public static int PlaySoundAt(Entity entity, string sound)
        {
            return PlaySoundAt(entity, sound, null);
        }

        public static int PlaySoundAt(Entity entity, string sound, string set)
        {
            throw new NotImplementedException("Native call bridge");
        }

        public static int PlaySoundAt(GTA.Math.Vector3 position, string sound)
        {
            return PlaySoundAt(position, sound, null);
        }

        public static int PlaySoundAt(GTA.Math.Vector3 position, string sound, string set)
        {
            throw new NotImplementedException("Native call bridge");
        }

        public static int PlaySoundFrontend(string sound)
        {
            return PlaySoundFrontend(sound, null);
        }

        public static int PlaySoundFrontend(string sound, string set)
        {
            throw new NotImplementedException("Native call bridge");
        }

        public static void StopSound(int id)
        {
            throw new NotImplementedException("Native call bridge");
        }

        public static void ReleaseSound(int id)
        {
            throw new NotImplementedException("Native call bridge");
        }

        public static bool HasSoundFinished(int id)
        {
            throw new NotImplementedException("Native call bridge");
        }

        public static void SetAudioFlag(AudioFlags flag, bool toggle)
        {
            throw new NotImplementedException("Native call bridge");
        }
    }
}
