#pragma once
#include "types.h"

class Crypto
{
public:
	static const int kSha1HashLen = 20;
	static const int kSha256HashLen = 32;
	static const int kAes128KeySize = 0x10;
	static const int kAesBlockSize = 0x10;
	static const int kRsa1024Size = 0x80;
	static const int kRsa2048Size = 0x100;
	static const int kRsa4096Size = 0x200;

	struct sRsa2048Key
	{
		u8 modulus[Crypto::kRsa2048Size];
		u8 priv_exponent[Crypto::kRsa2048Size];
	};

	static void Sha1(const u8* in, u32 size, u8 hash[kSha1HashLen]);
	static void Sha256(const u8* in, u32 size, u8 hash[kSha256HashLen]);

	static void AesCtr(const u8* in, u32 size, const u8 key[kAes128KeySize], u8 ctr[kAesBlockSize], u8* out);
	static void AesCbcDecrypt(const u8* in, u32 size, const u8 key[kAes128KeySize], u8 iv[kAesBlockSize], u8* out);
	static void AesCbcEncrypt(const u8* in, u32 size, const u8 key[kAes128KeySize], u8 iv[kAesBlockSize], u8* out);

	static int SignRsa2048Sha256(const u8 modulus[kRsa2048Size], const u8 private_exponent[kRsa2048Size], const u8 hash[kSha256HashLen], u8 signature[kRsa2048Size]);
	static int VerifyRsa2048Sha256(const u8 modulus[kRsa2048Size], const u8 hash[kSha256HashLen], const u8 signature[kRsa2048Size]);
};