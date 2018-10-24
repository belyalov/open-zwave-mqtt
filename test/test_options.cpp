#include <iostream>
#include <fstream>
#include <unistd.h>

#include <gtest/gtest.h>
#include <openzwave/platform/Log.h>
#include "options.h"

#define ARRAY_SIZE(a)   (sizeof(a) / sizeof(a[0]))

using namespace std;


TEST(options, full_names)
{
    const char* argv[] = {"exec_name",
                          "--config", "./",
                          "--system-config", "/dir/of/system/config",
                          "--device", "/dev/my_awesome_device",
                          "--mqtt_host", "localhost",
                          "--mqtt-port", "555",
                          "--mqtt-client_id", "mymqtt",
                          "--mqtt-prefix", "prefix",
                          "--mqtt_user", "junk",
                          "--mqtt-passwd", "secret",
                          "--log-level", "debug",
                          "--topic-filter-file", "1.txt",
                         };

    options opt;
    opt.parse_argv(ARRAY_SIZE(argv), argv);
    ASSERT_EQ("/dir/of/system/config", opt.system_config);
    ASSERT_EQ("./", opt.user_config);
    ASSERT_EQ("/dev/my_awesome_device", opt.device);
    ASSERT_EQ("localhost", opt.mqtt_host);
    ASSERT_EQ(555, opt.mqtt_port);
    ASSERT_EQ("mymqtt", opt.mqtt_client_id);
    ASSERT_EQ("prefix", opt.mqtt_prefix);
    ASSERT_EQ("junk", opt.mqtt_user);
    ASSERT_EQ("secret", opt.mqtt_passwd);
    ASSERT_EQ("1.txt", opt.topics_file);
    ASSERT_EQ(OpenZWave::LogLevel_Debug, opt.log_level);
}

TEST(options, log_level)
{
    map<string, uint32_t> runs = {
        {"error", OpenZWave::LogLevel_Error},
        {"warning", OpenZWave::LogLevel_Warning},
        {"info", OpenZWave::LogLevel_Info},
        {"debug", OpenZWave::LogLevel_Debug},
    };
    const char* argv[] = {"exec_name", "--log-level", ""};

    for (auto i = runs.begin(); i != runs.end(); ++i) {
        options opt;
        argv[2] = i->first.c_str();
        opt.parse_argv(ARRAY_SIZE(argv), argv);
        ASSERT_EQ(i->second, opt.log_level);
    }
}

TEST(options, short_names)
{
    const char* argv[] = {"exec_name",
                          "-c", "/dir/of/config",
                          "-d", "/dev/my_awesome_device",
                          "-h", "localhost",
                          "-u", "junk",
                          "-p", "secret",
                         };

    options opt;
    opt.parse_argv(ARRAY_SIZE(argv), argv);
    ASSERT_EQ("/dir/of/config", opt.user_config);
    ASSERT_EQ("/dev/my_awesome_device", opt.device);
    ASSERT_EQ("localhost", opt.mqtt_host);
    ASSERT_EQ("junk", opt.mqtt_user);
    ASSERT_EQ("secret", opt.mqtt_passwd);
}

TEST(options, invalid_options)
{
    const char* argv[] = {"exec_name", "-c", "/dir/of/config", "-Z", "fff"};

    options opt;
    ASSERT_THROW(opt.parse_argv(ARRAY_SIZE(argv), argv), param_error);
    ASSERT_EQ("/dir/of/config", opt.user_config);
}

TEST(options, value_required)
{
    const char* argv[] = {"exec_name", "-c"};

    options opt;
    ASSERT_THROW(opt.parse_argv(ARRAY_SIZE(argv), argv), param_error);
}

TEST(options, topic_overrides)
{
    // Create temp file with 3 topics
    ofstream f;
    f.open("1.txt");
    f << " # comment\n";
    f << "\n";
    f << "home/1/1=home/living\n";
    f << "garage/1/1\n";
    f << "hall/1    =   hall/lights\n";
    f.close();

    // Load / parse file
    options opts;
    opts.topics_file = "1.txt";
    opts.parse_topics_file();

    // Check result
    ASSERT_EQ(3, opts.topic_overrides.size());
    ASSERT_EQ(opts.topic_overrides["home/1/1"], "home/living");
    ASSERT_EQ(opts.topic_overrides["hall/1"], "hall/lights");
    ASSERT_EQ(opts.topic_overrides["garage/1/1"], "garage/1/1");

    unlink("1.txt");
}
