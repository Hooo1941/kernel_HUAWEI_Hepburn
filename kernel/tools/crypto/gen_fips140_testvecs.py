#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright 2021 Google LLC
#
# Generate most of the test vectors for the FIPS 140 cryptographic self-tests.
#
# Usage:
#    tools/crypto/gen_fips140_testvecs.py > crypto/fips140-generated-testvecs.h
#
# Prerequisites:
#    Debian:      apt-get install python3-pycryptodome python3-cryptography
#    Arch Linux:  pacman -S python-pycryptodomex python-cryptography

import hashlib
import hmac
import os

import Cryptodome.Cipher.AES
import Cryptodome.Util.Counter

import cryptography.hazmat.primitives.ciphers
import cryptography.hazmat.primitives.ciphers.algorithms
import cryptography.hazmat.primitives.ciphers.modes

scriptname = os.path.basename(__file__)

message     = bytes('This is a 32-byte test message.\0', 'ascii')
aes_key     = bytes('128-bit AES key\0', 'ascii')
aes_xts_key = bytes('This is an AES-128-XTS key.\0\0\0\0\0', 'ascii')
aes_iv      = bytes('ABCDEFGHIJKL\0\0\0\0', 'ascii')
assoc       = bytes('associated data string', 'ascii')
hmac_key    = bytes('128-bit HMAC key', 'ascii')

def warn_generated():
    print(f'''/*
 * This header was automatically generated by {scriptname}.
 * Don't edit it directly.
 */''')

def is_string_value(value):
    return (value.isascii() and
            all(c == '\x00' or c.isprintable() for c in str(value, 'ascii')))

def format_value(value, is_string):
    if is_string:
        return value
    hexstr = ''
    for byte in value:
        hexstr += f'\\x{byte:02x}'
    return hexstr

def print_value(name, value):
    is_string = is_string_value(value)
    hdr = f'static const u8 fips_{name}[{len(value)}] __initconst ='
    print(hdr, end='')
    if is_string:
        value = str(value, 'ascii').rstrip('\x00')
        chars_per_byte = 1
    else:
        chars_per_byte = 4
    bytes_per_line = 64 // chars_per_byte

    if len(hdr) + (chars_per_byte * len(value)) + 4 <= 80:
        print(f' "{format_value(value, is_string)}"', end='')
    else:
        for chunk in [value[i:i+bytes_per_line]
                      for i in range(0, len(value), bytes_per_line)]:
            print(f'\n\t"{format_value(chunk, is_string)}"', end='')
    print(';')
    print('')

def generate_aes_testvecs():
    print_value('aes_key', aes_key)
    print_value('aes_iv', aes_iv)

    cbc = Cryptodome.Cipher.AES.new(aes_key, Cryptodome.Cipher.AES.MODE_CBC,
                                    iv=aes_iv)
    print_value('aes_cbc_ciphertext', cbc.encrypt(message))

    ecb = Cryptodome.Cipher.AES.new(aes_key, Cryptodome.Cipher.AES.MODE_ECB)
    print_value('aes_ecb_ciphertext', ecb.encrypt(message))

    ctr = Cryptodome.Cipher.AES.new(aes_key, Cryptodome.Cipher.AES.MODE_CTR,
                                    nonce=aes_iv[:12])
    print_value('aes_ctr_ciphertext', ctr.encrypt(message))

    print_value('aes_gcm_assoc', assoc)
    gcm = Cryptodome.Cipher.AES.new(aes_key, Cryptodome.Cipher.AES.MODE_GCM,
                                    nonce=aes_iv[:12], mac_len=16)
    gcm.update(assoc)
    raw_ciphertext, tag = gcm.encrypt_and_digest(message)
    print_value('aes_gcm_ciphertext', raw_ciphertext + tag)

    # Unfortunately, pycryptodome doesn't support XTS, so for it we need to use
    # a different Python package (the "cryptography" package).
    print_value('aes_xts_key', aes_xts_key)
    xts = cryptography.hazmat.primitives.ciphers.Cipher(
        cryptography.hazmat.primitives.ciphers.algorithms.AES(aes_xts_key),
        cryptography.hazmat.primitives.ciphers.modes.XTS(aes_iv)).encryptor()
    ciphertext = xts.update(message) + xts.finalize()
    print_value('aes_xts_ciphertext', ciphertext)

def generate_sha_testvecs():
    print_value('hmac_key', hmac_key)
    for alg in ['sha1', 'sha256', 'hmac_sha256', 'sha512']:
        if alg.startswith('hmac_'):
            h = hmac.new(hmac_key, message, alg.removeprefix('hmac_'))
        else:
            h = hashlib.new(alg, message)
        print_value(f'{alg}_digest', h.digest())

print('/* SPDX-License-Identifier: GPL-2.0-only */')
print('/* Copyright 2021 Google LLC */')
print('')
warn_generated()
print('')
print_value('message', message)
generate_aes_testvecs()
generate_sha_testvecs()
warn_generated()