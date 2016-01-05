/**
 * @file
 *
 * @brief test suite for the crypto plugin
 *
 * @copyright BSD License (see doc/COPYING or http://www.libelektra.org)
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <kdb.h>
#include <tests_plugin.h>
#include <tests_internal.h>
#include <kdbinternal.h>
#include "crypto.h"

/*
 * The test vectors are taken from NIST SP 800-38A, section F.2.5 "CBC-AES256.Encrypt"
 * See <http://csrc.nist.gov/publications/nistpubs/800-38a/sp800-38a.pdf> for further information.
 */
static const unsigned char key[] =
{
	0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
	0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
	0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
	0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
};

static const unsigned char iv[] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};


/**
 * @brief create new KeySet and add a working configuration to it.
 */
static KeySet *getWorkingConfiguration()
{
	Key *configKey = keyNew("user/crypto/key-derivation/key", KEY_END);
	keySetBinary(configKey, key, sizeof(key));

	Key *configIv = keyNew("user/crypto/key-derivation/iv", KEY_END);
	keySetBinary(configIv, iv, sizeof(iv));

	return ksNew(2,
		configKey,
		configIv,
		KS_END);
}

/**
 * @brief create new KeySet and add an invalid configuration to it.
 *
 * The key in ks has an invalid size.
 */
static KeySet *getInvalidConfiguration()
{
	const unsigned char wrongKey[] = { 0x01, 0x02, 0x03 };

	Key *configKey = keyNew("user/crypto/key-derivation/key", KEY_END);
	keySetBinary(configKey, wrongKey, sizeof(wrongKey));

	Key *configIv = keyNew("user/crypto/key-derivation/iv", KEY_END);
	keySetBinary(configIv, iv, sizeof(iv));

	return ksNew(2,
		configKey,
		configIv,
		KS_END);
}

/**
 * @brief create new KeySet and add an incomplete configuration to it.
 *
 * The required key "/elektra/modules/crypto/key-derivation/iv" is missing.
 */
static KeySet *getIncompleteConfiguration()
{
	Key *configKey = keyNew("proc/elektra/modules/crypto/key-derivation/key", KEY_END);
	keySetBinary(configKey, key, sizeof(key));

	return ksNew(1,
		configKey,
		KS_END);
}

static void test_init_internal(Plugin *plugin, Key *parentKey)
{
	KeySet *config = elektraPluginGetConfig (plugin);
	succeed_if (config != 0, "there should be a config");

	succeed_if (plugin->kdbOpen != 0, "no open pointer");
	succeed_if (plugin->kdbClose != 0, "no close pointer");
	succeed_if (plugin->kdbGet != 0, "no get pointer");
	succeed_if (plugin->kdbSet != 0, "no set pointer");
	succeed_if (plugin->kdbError!= 0, "no error pointer");

	// try re-opening the plugin
	succeed_if (plugin->kdbClose(plugin, parentKey) == 1, "kdb close failed");
	succeed_if (plugin->kdbOpen(plugin, parentKey) == 1, "re-opening the plugin failed");
	succeed_if (plugin->kdbClose(plugin, parentKey) == 1, "kdb close failed");

	elektraPluginClose(plugin, 0);
}

static void test_init()
{
	Plugin *plugin = NULL;
	Key *parentKey = keyNew("system", KEY_END);
	KeySet *modules = ksNew(0, KS_END);
	elektraModulesInit (modules, 0);

	plugin = elektraPluginOpen ("crypto_gcrypt", modules, getWorkingConfiguration(), 0);
	if (plugin)
	{
		test_init_internal (plugin, parentKey);
		succeed_if (!strcmp(plugin->name, "crypto_gcrypt"), "got wrong name");
	}

	plugin = elektraPluginOpen ("crypto_openssl", modules, getWorkingConfiguration(), 0);
	if (plugin)
	{
		test_init_internal (plugin, parentKey);
		succeed_if (!strcmp(plugin->name, "crypto_openssl"), "got wrong name");
	}

	plugin = elektraPluginOpen ("crypto", modules, getWorkingConfiguration(), 0);
	exit_if_fail (plugin, "could not load crypto_openssl plugin");
	test_init_internal (plugin, parentKey);
	succeed_if (!strcmp(plugin->name, "crypto"), "got wrong name");

	elektraModulesClose(modules, 0);
	ksDel (modules);
	keyDel (parentKey);
}

static void test_config_errors()
{
	Plugin *plugin = NULL;
	Key *parentKey = keyNew("system", KEY_END);
	KeySet *modules = ksNew(0, KS_END);
	elektraModulesInit (modules, 0);

	// gcrypt tests
	plugin = elektraPluginOpen ("crypto_gcrypt", modules, getWorkingConfiguration(), 0);
	if (plugin)
	{
		succeed_if (plugin->kdbGet(plugin, 0, parentKey) == 1, "kdb get failed with valid config");
		elektraPluginClose(plugin, 0);
	}

	plugin = elektraPluginOpen ("crypto_gcrypt", modules, getInvalidConfiguration(), 0);
	if (plugin)
	{
		succeed_if (plugin->kdbGet(plugin, 0, parentKey) != 1, "kdb get succeeded with invalid config");
		elektraPluginClose(plugin, 0);
	}

	plugin = elektraPluginOpen ("crypto_gcrypt", modules, getIncompleteConfiguration(), 0);
	if (plugin)
	{
		succeed_if (plugin->kdbGet(plugin, 0, parentKey) != 1, "kdb get succeeded with incomplete config");
		elektraPluginClose(plugin, 0);
	}

	// OpenSSL tests
	plugin = elektraPluginOpen ("crypto_openssl", modules, getWorkingConfiguration(), 0);
	if (plugin)
	{
		succeed_if (plugin->kdbGet(plugin, 0, parentKey) == 1, "kdb get failed with valid config");
		elektraPluginClose(plugin, 0);
	}

	plugin = elektraPluginOpen ("crypto_openssl", modules, getInvalidConfiguration(), 0);
	if (plugin)
	{
		succeed_if (plugin->kdbGet(plugin, 0, parentKey) != 1, "kdb get succeeded with invalid config");
		elektraPluginClose(plugin, 0);
	}

	plugin = elektraPluginOpen ("crypto_openssl", modules, getIncompleteConfiguration(), 0);
	if (plugin)
	{
		succeed_if (plugin->kdbGet(plugin, 0, parentKey) != 1, "kdb get succeeded with incomplete config");
		elektraPluginClose(plugin, 0);
	}


	elektraModulesClose(modules, 0);
	ksDel (modules);
	keyDel (parentKey);
}

int main(int argc, char** argv)
{
	printf("CYPTO        TESTS\n");
	printf("==================\n\n");

	init(argc, argv);

	test_init();
	test_config_errors();

	printf("\ntestmod_crypto RESULTS: %d test(s) done. %d error(s).\n", nbTest, nbError);
	return nbError;
}

