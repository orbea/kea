// Copyright (C) 2011  Internet Systems Consortium, Inc. ("ISC")
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <config.h>
#include <gtest/gtest.h>

#include <crypto/crypto.h>
#include <crypto/crypto_botan.h>
#include <dns/buffer.h>
#include <exceptions/exceptions.h>

using namespace isc::dns;
using namespace isc::crypto;

TEST(CryptoTest, HMAC_SIGN) {
    char data_b[] = "Hi there";
    OutputBuffer data(8);
    data.writeData(data_b, 8);

	TSIGKey key = TSIGKeyFromString("test.example:MSG6Ng==:hmac-md5.sig-alg.reg.int");

    OutputBuffer hmac_sig(1);
    
    doHMAC(data, key, hmac_sig);
    bool result = verifyHMAC(data, key, hmac_sig);
    EXPECT_TRUE(result);
}

TEST(CryptoText, TSIGKeyFromString) {
	TSIGKey k1 = TSIGKeyFromString("test.example:MSG6Ng==:hmac-md5.sig-alg.reg.int");
	TSIGKey k2 = TSIGKeyFromString("test.example.:MSG6Ng==:hmac-md5.sig-alg.reg.int.");
	TSIGKey k3 = TSIGKeyFromString("test.example:MSG6Ng==");
	
	EXPECT_EQ("test.example.:MSG6Ng==:hmac-md5.sig-alg.reg.int.",
	          TSIGKeyToString(k1));
	EXPECT_EQ("test.example.:MSG6Ng==:hmac-md5.sig-alg.reg.int.",
	          TSIGKeyToString(k2));
	EXPECT_EQ("test.example.:MSG6Ng==:hmac-md5.sig-alg.reg.int.",
	          TSIGKeyToString(k3));

	EXPECT_THROW(TSIGKeyFromString(""), isc::InvalidParameter);
	EXPECT_THROW(TSIGKeyFromString("::"), isc::InvalidParameter);
	EXPECT_THROW(TSIGKeyFromString("test.example.::"), isc::InvalidParameter);
	EXPECT_THROW(TSIGKeyFromString("test.example.:MSG6Ng==:unknown"), isc::InvalidParameter);
}
