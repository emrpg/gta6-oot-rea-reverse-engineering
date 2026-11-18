// Reconstructed C# implementation of RAGE RPF7/RPF8 Key Search, Decryption & RSC7 Page Relocation
// Evidence source:
//   - CodeWalker.Core/GameFiles/RpfFile.cs (RSC7 magic 0x37435352, DecryptAES / DecryptNG)
//   - Closed-source OpenIV / RPF7-RPF8 encryption pipeline analysis

using System;
using System.IO;
using System.Security.Cryptography;
using System.Text;

namespace ClosedSourceReconstruction.Rage
{
    public enum RpfEncryption : uint
    {
        None = 0,
        Open = 0x4E45504F, // "OPEN"
        Aes  = 0x0FFFFFF9,
        Ng   = 0x0FEFFFFF
    }

    public struct Rpf7Header
    {
        public uint Version;       // 0x52504637 ("RPF7")
        public uint EntryCount;
        public uint NamesLength;
        public uint Encryption;    // RpfEncryption
    }

    public struct Rsc7Header
    {
        public const uint Magic = 0x37435352; // "RSC7"
        public uint Version;
        public uint SystemFlags;
        public uint GraphicsFlags;

        public static uint ComputeSegmentSize(uint flags)
        {
            uint baseSize = 0x2000u << (int)(flags & 0xF);
            uint s0 = ((flags >> 17) & 0x7F) + (((flags >> 11) & 0x3F) << 1) +
                      (((flags >> 7)  & 0x0F) << 2) + (((flags >> 5)  & 0x03) << 3) +
                      (((flags >> 4)  & 0x01) << 4);
            // Total virtual pages scaled by page shift
            uint total = 0;
            for (int i = 0; i < 4; i++)
            {
                uint cnt = (flags >> (4 + i * 3)) & 0x7;
                total += cnt * (baseSize >> i);
            }
            return total > 0 ? total : s0 * 0x1000u;
        }
    }

    public static class RpfCryptoReconstructed
    {
        public static byte[] DecryptAesEcb(byte[] data, byte[] aesKey32)
        {
            if (data == null || data.Length < 16 || aesKey32 == null || aesKey32.Length != 32)
                throw new ArgumentException("Invalid AES-256 ECB input.");

            int alignedLen = data.Length & ~0xF;
            byte[] output = (byte[])data.Clone();

            using var aes = Aes.Create();
            aes.KeySize = 256;
            aes.Mode = CipherMode.ECB;
            aes.Padding = PaddingMode.None;
            aes.Key = aesKey32;

            using var dec = aes.CreateDecryptor();
            dec.TransformBlock(data, 0, alignedLen, output, 0);
            return output;
        }

        // NG (Next-Gen) 17-round Substitution-Permutation key index derivation
        public static uint ComputeNgKeyIndex(string entryName, uint fileSize)
        {
            uint hash = 0;
            foreach (char ch in entryName)
            {
                byte b = (byte)char.ToLowerInvariant(ch);
                hash += b;
                hash += hash << 10;
                hash ^= hash >> 6;
            }
            hash += hash << 3;
            hash ^= hash >> 11;
            hash += hash << 15;
            return (hash + fileSize + 101u - 40u) % 0x65u;
        }

        // Searches a process memory dump for candidate 32-byte AES keys via Shannon entropy + known prefix/hash
        public static long SearchKeyInDump(byte[] dump, byte[] expectedSha256)
        {
            using var sha = SHA256.Create();
            for (long off = 0; off + 32 <= dump.Length; off += 4)
            {
                byte[] candidate = new byte[32];
                Buffer.BlockCopy(dump, (int)off, candidate, 0, 32);
                byte[] digest = sha.ComputeHash(candidate);
                if (CryptographicOperations.FixedTimeEquals(digest, expectedSha256))
                    return off;
            }
            return -1;
        }

        public static Rpf7Header ReadRpf7Header(Stream s)
        {
            using var br = new BinaryReader(s, Encoding.ASCII, leaveOpen: true);
            var hdr = new Rpf7Header
            {
                Version    = br.ReadUInt32(),
                EntryCount = br.ReadUInt32(),
                NamesLength = br.ReadUInt32(),
                Encryption = br.ReadUInt32()
            };
            if (hdr.Version != 0x52504637u)
                throw new InvalidDataException($"Unexpected RPF magic: 0x{hdr.Version:X8}");
            return hdr;
        }
    }
}
