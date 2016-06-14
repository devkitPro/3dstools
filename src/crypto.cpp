#include "crypto.h"
#include "polarssl/aes.h"
#include "polarssl/sha1.h"
#include "polarssl/sha2.h"
#include "polarssl/rsa.h"

void Crypto::Sha1(const u8* in, u32 size, u8 hash[kSha1HashLen])
{
	sha1(in, size, hash);
}

void Crypto::Sha256(const u8* in, u32 size, u8 hash[kSha256HashLen])
{
	sha2(in, size, hash, false);
}

void Crypto::AesCtr(const u8* in, u32 size, const u8 key[kAes128KeySize], u8 ctr[kAesBlockSize], u8* out)
{
	aes_context ctx;
	u8 block[kAesBlockSize] = { 0 };
	size_t counterOffset = 0;

	aes_setkey_enc(&ctx, key, 128);
	aes_crypt_ctr(&ctx, size, &counterOffset, ctr, block, in, out);
}

void Crypto::AesCbcDecrypt(const u8* in, u32 size, const u8 key[kAes128KeySize], u8 iv[kAesBlockSize], u8* out)
{
	aes_context ctx;
	aes_setkey_dec(&ctx, key, 128);
	aes_crypt_cbc(&ctx, AES_DECRYPT, size, iv, in, out);
}

void Crypto::AesCbcEncrypt(const u8* in, u32 size, const u8 key[kAes128KeySize], u8 iv[kAesBlockSize], u8* out)
{
	aes_context ctx;
	aes_setkey_enc(&ctx, key, 128);
	aes_crypt_cbc(&ctx, AES_ENCRYPT, size, iv, in, out);
}

int Crypto::SignRsa2048Sha256(const u8 modulus[kRsa2048Size], const u8 private_exponent[kRsa2048Size], const u8 hash[kSha256HashLen], u8 signature[kRsa2048Size])
{
	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V15, 0);

	ctx.len = kRsa2048Size;
	mpi_read_binary(&ctx.D, private_exponent, ctx.len);
	mpi_read_binary(&ctx.N, modulus, ctx.len);

	ret = rsa_rsassa_pkcs1_v15_sign(&ctx, RSA_PRIVATE, SIG_RSA_SHA256, kSha256HashLen, hash, signature);

	rsa_free(&ctx);

	return ret;
}

int Crypto::VerifyRsa2048Sha256(const u8 modulus[kRsa2048Size], const u8 hash[kSha256HashLen], const u8 signature[kRsa2048Size])
{
	static const u8 public_exponent[3] = { 0x01, 0x00, 0x01 };

	int ret;
	rsa_context ctx;
	rsa_init(&ctx, RSA_PKCS_V15, 0);

	ctx.len = kRsa2048Size;
	mpi_read_binary(&ctx.E, public_exponent, sizeof(public_exponent));
	mpi_read_binary(&ctx.N, modulus, ctx.len);

	ret = rsa_rsassa_pkcs1_v15_verify(&ctx, RSA_PUBLIC, SIG_RSA_SHA256, kSha256HashLen, hash, signature);

	rsa_free(&ctx);

	return ret;
}
