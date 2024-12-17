/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * sha256_core.c
 *
 * sha256
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "fl_type.h"
#include "sha256_core.h"

/* Initial Hash Values: FIPS 180-3 section 5.3.3 */
static const uint32_t g_sha256_hiv[8] = {
	0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
	0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
};

/*
 * SHA224_256ProcessMessageBlock
 *
 * Description:
 *   This helper function will process the next 512 bits of the
 *   message stored in the message_block array.
 *
 * Parameters:
 *   context: [in/out]
 *     The SHA context to update.
 *
 * Returns:
 *   Nothing.
 *
 * Comments:
 *   Many of the variable names in this code, especially the
 *   single character names, were used because those were the
 *   names used in the Secure Hash Standard.
 */
/* Constants defined in FIPS 180-3, section 4.2.2 */
static const uint32_t g_sha256_key[] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b,
	0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
	0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7,
	0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152,
	0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
	0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
	0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819,
	0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
	0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f,
	0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

uint32_t sha256_shr(uint32_t bits, uint32_t word)
{
	return  (word >> bits);
}

uint32_t sha256_rotl(uint32_t bits, uint32_t word)
{
	return  ((word << bits) | (word >> (32 - bits)));
}

uint32_t sha256_rotr(uint32_t bits, uint32_t word)
{
	return  ((word >> bits) | (word << (32 - bits)));
}

uint32_t sha256_sigma_1(uint32_t word)
{
	return (sha256_rotr(2, word) ^
		sha256_rotr(13, word) ^ sha256_rotr(22, word));
}

uint32_t sha256_sigma_2(uint32_t word)
{
	return (sha256_rotr(6, word) ^
		sha256_rotr(11, word) ^ sha256_rotr(25, word));
}

uint32_t sha256_sigma_3(uint32_t word)
{
	return (sha256_rotr(7, word) ^
		sha256_rotr(18, word) ^ sha256_shr(3, word));
}

uint32_t sha256_sigma_4(uint32_t word)
{
	return (sha256_rotr(17, word) ^
		sha256_rotr(19, word) ^ sha256_shr(10, word));
}

uint32_t sha_ch(uint32_t x, uint32_t y, uint32_t z)
{
	return  ((x & (y ^ z)) ^ z);
}

uint32_t sha_maj(uint32_t x, uint32_t y, uint32_t z)
{
	return  ((x & (y | z)) | (y & z));
}

static void get_u32_be(uint32_t *pt_n, uint8_t *b, uint32_t offset)
{
	*pt_n = (((uint32_t) b[offset]) << 24) |
		(((uint32_t) b[offset + 1]) << 16) |
		(((uint32_t) b[offset + 2]) <<  8) |
		(((uint32_t) b[offset + 3]));
}

static void sha256_process_messageblock(struct str_sha256_context *context)
{
	int t;
	uint32_t temp1;
	uint32_t temp2;
	uint32_t word_sequence[SHA256_MESSAGE_BLOCK_SIZE];
	uint32_t a[8];

	/* Initialize the first 16 words in the array W */
	for (t = 0; t < 16; t++)
		get_u32_be(word_sequence + t, context->message_block, t * 4);

	for (t = 16; t < 64; t++)
		word_sequence[t] = sha256_sigma_4(word_sequence[t - 2]) +
			word_sequence[t - 7] +
			sha256_sigma_3(word_sequence[t - 15]) +
			word_sequence[t - 16];

	a[0] = context->state[0];
	a[1] = context->state[1];
	a[2] = context->state[2];
	a[3] = context->state[3];
	a[4] = context->state[4];
	a[5] = context->state[5];
	a[6] = context->state[6];
	a[7] = context->state[7];

	for (t = 0; t < SHA256_MESSAGE_BLOCK_SIZE; t++) {
		temp1 = a[7] + sha256_sigma_2(a[4]) + sha_ch(a[4], a[5], a[6]) +
			g_sha256_key[t] + word_sequence[t];
		temp2 = sha256_sigma_1(a[0]) + sha_maj(a[0], a[1], a[2]);
		a[7] = a[6];
		a[6] = a[5];
		a[5] = a[4];
		a[4] = a[3] + temp1;
		a[3] = a[2];
		a[2] = a[1];
		a[1] = a[0];
		a[0] = temp1 + temp2;
	}

	context->state[0] += a[0];
	context->state[1] += a[1];
	context->state[2] += a[2];
	context->state[3] += a[3];
	context->state[4] += a[4];
	context->state[5] += a[5];
	context->state[6] += a[6];
	context->state[7] += a[7];

	context->message_block_index = 0;
}

/*
 * sha256_pad_message
 *
 * Description:
 *   According to the standard, the message must be padded to the next
 *   even multiple of 512 bits.  The first padding bit must be a '1'.
 *   The last 64 bits represent the length of the original message.
 *   All bits in between should be 0.  This helper function will pad
 *   the message according to those rules by filling the
 *   message_block array accordingly.  When it returns, it can be
 *   assumed that the message digest has been computed.
 *
 * Parameters:
 *   context: [in/out]
 *     The context to pad.
 *   Pad_Byte: [in]
 *     The last byte to add to the message block before the 0-padding
 *     and length.  This will contain the last bits of the message
 *     followed by another single bit.  If the message was an
 *     exact multiple of 8-bits long, Pad_Byte will be 0x80.
 *
 * Returns:
 *   Nothing.
 */
static void sha256_pad_message(struct str_sha256_context *context,
		uint8_t pad_byte)
{
	/*
	 * Check to see if the current message block is too small to hold
	 * the initial padding bits and length.  If so, we will pad the
	 * block, process it, and then continue padding into a second
	 * block.
	 */
	if (context->message_block_index >= (SHA256_MESSAGE_BLOCK_SIZE - 8)) {
		context->message_block[context->message_block_index++] = pad_byte;
		while (context->message_block_index < SHA256_MESSAGE_BLOCK_SIZE)
			context->message_block[context->message_block_index++] = 0;
		sha256_process_messageblock(context);
	} else {
		context->message_block[context->message_block_index++] = pad_byte;
	}

	while (context->message_block_index < (SHA256_MESSAGE_BLOCK_SIZE - 8))
		context->message_block[context->message_block_index++] = 0;

	/*
	 * Store the message length as the last 8 octets
	 */
	context->message_block[56] = (uint8_t)(context->length_high >> 24);
	context->message_block[57] = (uint8_t)(context->length_high >> 16);
	context->message_block[58] = (uint8_t)(context->length_high >> 8);
	context->message_block[59] = (uint8_t)(context->length_high);
	context->message_block[60] = (uint8_t)(context->length_low >> 24);
	context->message_block[61] = (uint8_t)(context->length_low >> 16);
	context->message_block[62] = (uint8_t)(context->length_low >> 8);
	context->message_block[63] = (uint8_t)(context->length_low);

	sha256_process_messageblock(context);
}

/*
 * sha256_finalize
 *
 * Description:
 *   This helper function finishes off the digest calculations.
 *
 * Parameters:
 *   context: [in/out]
 *     The SHA context to update.
 *   Pad_Byte: [in]
 *     The last byte to add to the message block before the 0-padding
 *     and length.  This will contain the last bits of the message
 *     followed by another single bit.  If the message was an
 *     exact multiple of 8-bits long, Pad_Byte will be 0x80.
 *
 * Returns:
 *   sha Error Code.
 */
static void sha256_finalize(struct str_sha256_context *context,
		uint8_t pad_byte)
{
	int i;

	sha256_pad_message(context, pad_byte);
	/* message may be sensitive, so clear it out */
	for (i = 0; i < SHA256_MESSAGE_BLOCK_SIZE; ++i)
		context->message_block[i] = 0;
	context->length_high = 0; /* and clear length */
	context->length_low  = 0;
	context->computed    = 1;
}

/*
 * sha256_resetiv
 *
 * Description:
 *   This helper function will initialize the struct str_sha256_context in
 *   preparation for computing a new SHA-224 or SHA-256 message digest.
 *
 * Parameters:
 *   context: [in/out]
 *     The context to reset.
 *   h_in[ ]: [in]
 *     The initial hash value array to use.
 *
 * Returns:
 *   sha Error Code.
 */
static int sha256_resetiv(struct str_sha256_context *context, uint32_t *h_in)
{
	if (!context)
		return SHA_NULL;

	context->length_high = context->length_low = 0;
	context->message_block_index = 0;

	context->state[0] = h_in[0];
	context->state[1] = h_in[1];
	context->state[2] = h_in[2];
	context->state[3] = h_in[3];
	context->state[4] = h_in[4];
	context->state[5] = h_in[5];
	context->state[6] = h_in[6];
	context->state[7] = h_in[7];

	context->computed  = 0;
	context->corrupted = SHA_SUCCESS;

	return SHA_SUCCESS;
}

/*
 * sha256_result_n
 *
 * Description:
 *   This helper function will return the 224-bit or 256-bit message
 *   digest into the Message_Digest array provided by the caller.
 *   NOTE:
 *    The first octet of hash is stored in the element with index 0,
 *    the last octet of hash in the element with index 27/31.
 *
 * Parameters:
 *   context: [in/out]
 *     The context to use to calculate the SHA hash.
 *   Message_Digest[ ]: [out]
 *     Where the digest is returned.
 *   HashSize: [in]
 *     The size of the hash, either 28 or 32.
 *
 * Returns:
 *   sha Error Code.
 */
static int sha256_result_n(struct str_sha256_context *context,
		uint8_t *p_digest, int hash_size)
{
	int i;

	if (!context)
		return SHA_NULL;
	if (!p_digest)
		return SHA_NULL;
	if (context->corrupted)
		return context->corrupted;

	if (!context->computed)
		sha256_finalize(context, SHA256_PAD_BYTE);

	for (i = 0; i < hash_size; ++i)
		p_digest[i] =
			(uint8_t)(context->state[i >> 2] >> 8 * (3 - (i & 0x03)));

	return SHA_SUCCESS;
}

/*
 * sha256_reset
 *
 * Description:
 *  This function will initialize the struct str_sha256_context in preparation
 *  for computing a new SHA256 message digest.
 *
 * Parameters:
 *   context: [in/out]
 *     The context to reset.
 *
 * Returns:
 *   sha Error Code.
 */
int sha256_reset(struct str_sha256_context *context)
{
	return sha256_resetiv(context, (uint32_t *)g_sha256_hiv);
}

/*
 * sha256_input
 *
 * Description:
 *   This function accepts an array of octets as the next portion
 *   of the message.
 *
 * Parameters:
 *   context: [in/out]
 *     The SHA context to update.
 *   message_array[ ]: [in]
 *     An array of octets representing the next portion of
 *     the message.
 *   length: [in]
 *     The length of the message in message_array.
 *
 * Returns:
 *   sha Error Code.
 */
int sha256_input(struct str_sha256_context *context,
		const uint8_t *message_array, uint32_t length)
{
	uint32_t add_temp;

	if (!context)
		return SHA_NULL;
	if (!length)
		return SHA_SUCCESS;
	if (!message_array)
		return SHA_NULL;
	if (context->computed)
		return context->corrupted = SHA_STATEERROR;
	if (context->corrupted)
		return context->corrupted;

	while (length--) {
		context->message_block[context->message_block_index++] =
			*message_array;
		add_temp = context->length_low;
		context->length_low += 8;
		if (context->length_low < add_temp) {
			context->length_high += 1;
			if (context->length_high == 0)
				context->corrupted = SHA_INTOOLONG;
		}

		if ((context->corrupted == SHA_SUCCESS) &&
			(context->message_block_index == SHA256_MESSAGE_BLOCK_SIZE))
			sha256_process_messageblock(context);

		message_array++;
	}

	return context->corrupted;
}

/*
 * sha256_result
 *
 * Description:
 *   This function will return the 256-bit message digest
 *   into the Message_Digest array provided by the caller.
 *   NOTE:
 *    The first octet of hash is stored in the element with index 0,
 *    the last octet of hash in the element with index 31.
 *
 * Parameters:
 *   context: [in/out]
 *     The context to use to calculate the SHA hash.
 *   Message_Digest[ ]: [out]
 *     Where the digest is returned.
 *
 * Returns:
 *   sha Error Code.
 */
int sha256_result(struct str_sha256_context *context, uint8_t *p_digest)
{
	return sha256_result_n(context, p_digest, SHA256_HASHSIZE);
}

uint8_t fl_sha256_init(struct str_sha256_context *ctx)
{
	ctx->alg = ALG_SHA_256;
	return sha256_reset(ctx);
}

uint8_t fl_sha256_update(struct str_sha256_context *ctx,
		const uint8_t *in_buff, uint32_t msglen)
{
	return sha256_input(ctx, in_buff, msglen);
}

uint8_t fl_sha256_final(struct str_sha256_context *ctx, uint8_t *p_digest)
{
	return sha256_result(ctx, p_digest);
}
