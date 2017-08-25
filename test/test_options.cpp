
#include <gtest/gtest.h>
#include "options.h"

TEST(options, full_names)
{
    const char* argv[] = {"exec_name",
                          "--debug",
                          "--config", "/dir/of/config",
                          "--device", "/dev/my_awesome_device",
                          "--mqtt_host", "localhost",
                          "--mqtt-port", "555",
                          "--mqtt-client_id", "mymqtt",
                          "--mqtt-prefix", "prefix",
                          "--mqtt_user", "junk",
                          "--mqtt-passwd", "secret",
                         };

    options opt;
    bool res = opt.parse_argv(18, argv);
    ASSERT_TRUE(res);
    ASSERT_EQ("/dir/of/config", opt.openzwave_config);
    ASSERT_EQ("/dev/my_awesome_device", opt.device);
    ASSERT_EQ("localhost", opt.mqtt_host);
    ASSERT_EQ(555, opt.mqtt_port);
    ASSERT_EQ("mymqtt", opt.mqtt_client_id);
    ASSERT_EQ("prefix", opt.mqtt_prefix);
    ASSERT_EQ("junk", opt.mqtt_user);
    ASSERT_EQ("secret", opt.mqtt_passwd);
    ASSERT_TRUE(opt.debug);
}

TEST(options, short_names)
{
    const char* argv[] = {"exec_name",
                          "-c", "/dir/of/config",
                          "-d", "/dev/my_awesome_device",
                          "-h", "localhost",
                          "-p", "555",
                          "-C", "mymqtt",
                          "-u", "junk",
                          "-P", "secret",
                          "-D",
                         };

    options opt;
    bool res = opt.parse_argv(16, argv);
    ASSERT_TRUE(res);
    ASSERT_EQ("/dir/of/config", opt.openzwave_config);
    ASSERT_EQ("/dev/my_awesome_device", opt.device);
    ASSERT_EQ("localhost", opt.mqtt_host);
    ASSERT_EQ(555, opt.mqtt_port);
    ASSERT_EQ("mymqtt", opt.mqtt_client_id);
    ASSERT_EQ("junk", opt.mqtt_user);
    ASSERT_EQ("secret", opt.mqtt_passwd);
}

TEST(options, invalid_options)
{
    const char* argv[] = {"exec_name", "-c", "/dir/of/config", "-Z", "fff"};

    options opt;
    bool res = opt.parse_argv(5, argv);
    ASSERT_FALSE(res);
    ASSERT_EQ("/dir/of/config", opt.openzwave_config);
}

TEST(options, value_required)
{
    const char* argv[] = {"exec_name", "--debug", "-c"};

    options opt;
    bool res = opt.parse_argv(3, argv);
    ASSERT_FALSE(res);
    ASSERT_TRUE(opt.debug);
}

