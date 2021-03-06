/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2013 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include "config.h"
#include <gtest/gtest.h>
#include <libcouchbase/couchbase.h>
#include "hostlist.h"

class Hostlist : public ::testing::Test
{
};

static bool hostEquals(const lcb_host_t &host, const char *addr, const char *port)
{
    return strcmp(host.host, addr) == 0 && strcmp(host.port, port) == 0;
}

TEST_F(Hostlist, testParseBasic)
{
    lcb_host_t curhost;
    lcb_error_t err;

    err = lcb_host_parsez(&curhost, "1.2.3.4", 8091);
    ASSERT_EQ(LCB_SUCCESS, err);
    ASSERT_TRUE(hostEquals(curhost, "1.2.3.4", "8091"));

    err = lcb_host_parsez(&curhost, "1.2.3.4:9000", 8091);
    ASSERT_EQ(LCB_SUCCESS, err);
    ASSERT_TRUE(hostEquals(curhost, "1.2.3.4", "9000"));

    err = lcb_host_parsez(&curhost, "http://1.2.3.4:900/pools/default", 8091);
    ASSERT_EQ(LCB_SUCCESS, err);
    ASSERT_TRUE(hostEquals(curhost, "1.2.3.4", "900"));

    err = lcb_host_parsez(&curhost, "", 1000);
    ASSERT_EQ(LCB_INVALID_HOST_FORMAT, err);

    err = lcb_host_parsez(&curhost, "foo.com", -1);
    ASSERT_EQ(LCB_INVALID_HOST_FORMAT, err);

    err = lcb_host_parsez(&curhost, "foo.com:", 100);
    ASSERT_EQ(LCB_INVALID_HOST_FORMAT, err);

    err = lcb_host_parsez(&curhost, "localhost/foo", 100);
    ASSERT_EQ(LCB_SUCCESS, err);

    err = lcb_host_parsez(&curhost, "localhost:1111111111111111111111111111", 100);
    ASSERT_EQ(LCB_INVALID_HOST_FORMAT, err);
}


TEST_F(Hostlist, testEquals)
{
    lcb_host_t host_a, host_b;
    strcpy(host_a.host, "foo.com");
    strcpy(host_a.port, "1234");
    strcpy(host_b.host, "foo.com");
    strcpy(host_b.port, "1234");
    ASSERT_NE(0, lcb_host_equals(&host_a, &host_b));

    strcpy(host_a.host, "bar.com");
    ASSERT_EQ(0, lcb_host_equals(&host_a, &host_b));


    strcpy(host_a.host, "foo.com");
    strcpy(host_a.port, "44444");
    ASSERT_EQ(0, lcb_host_equals(&host_a, &host_b));
}

TEST_F(Hostlist, testParseList)
{
    hostlist_t hosts = hostlist_create();
    ASSERT_FALSE(NULL == hosts);

    lcb_error_t err;
    err = hostlist_add_stringz(hosts, "1.1.1.1",  8091);
    ASSERT_EQ(LCB_SUCCESS, err);
    ASSERT_EQ(1, hosts->size());
    ASSERT_TRUE(hosts->exists("1.1.1.1:8091"));


    hostlist_clear(hosts);
    err = hostlist_add_stringz(hosts, "1.1.1.1;",  8091);
    ASSERT_EQ(LCB_SUCCESS, err);
    ASSERT_EQ(1, hosts->size());
    ASSERT_TRUE(hosts->exists("1.1.1.1:8091"));

    hostlist_clear(hosts);
    err = hostlist_add_stringz(hosts, ";",  8091);
    ASSERT_EQ(LCB_SUCCESS, err);
    ASSERT_EQ(0, hosts->size());

    hostlist_clear(hosts);
    err = hostlist_add_stringz(hosts, ";;;;",  8091);
    ASSERT_EQ(LCB_SUCCESS, err);
    ASSERT_EQ(0, hosts->size());

    hostlist_clear(hosts);
    err = hostlist_add_stringz(hosts, "1.1.1.1;2.2.2.2",  8091);
    ASSERT_EQ(LCB_SUCCESS, err);
    ASSERT_EQ(2, hosts->size());
    ASSERT_TRUE(hosts->exists("1.1.1.1:8091"));
    ASSERT_TRUE(hosts->exists("2.2.2.2:8091"));


    hostlist_clear(hosts);
    err = hostlist_add_stringz(hosts, "1.1.1.1:1000;2.2.2.2:2000;3.3.3.3", 8091);
    ASSERT_EQ(LCB_SUCCESS, err);
    ASSERT_EQ(3, hosts->size());
    ASSERT_TRUE(hosts->exists("1.1.1.1:1000"));
    ASSERT_TRUE(hosts->exists("2.2.2.2:2000"));
    ASSERT_TRUE(hosts->exists("3.3.3.3:8091"));

    hostlist_clear(hosts);
    err = hostlist_add_stringz(hosts, "1.1.1.1;1.1.1.1;1.1.1.1", 8091);
    ASSERT_EQ(LCB_SUCCESS, err);
    ASSERT_EQ(1, hosts->size());
    ASSERT_TRUE(hosts->exists("1.1.1.1:8091"));


    hostlist_clear(hosts);
    err = hostlist_add_stringz(hosts, "1.1.1.1:9000;1.1.1.1:9001;1.1.1.1:9002",  8091);
    ASSERT_EQ(LCB_SUCCESS, err);
    ASSERT_EQ(3, hosts->size());
    ASSERT_TRUE(hosts->exists("1.1.1.1:9000"));
    ASSERT_TRUE(hosts->exists("1.1.1.1:9001"));
    ASSERT_TRUE(hosts->exists("1.1.1.1:9002"));

    hostlist_clear(hosts);
    ASSERT_EQ(LCB_SUCCESS, hostlist_add_stringz(hosts, "1.1.1.1", 8091));
    ASSERT_EQ(LCB_SUCCESS, hostlist_add_stringz(hosts, "2.2.2.2", 8091));
    ASSERT_EQ(LCB_SUCCESS, hostlist_add_stringz(hosts, "3.3.3.3", 8091));
    ASSERT_EQ(3, hosts->size());

    ASSERT_TRUE(hosts->exists("1.1.1.1:8091"));
    ASSERT_TRUE(hosts->exists("2.2.2.2:8091"));
    ASSERT_TRUE(hosts->exists("3.3.3.3:8091"));

    hostlist_randomize(hosts);
    hostlist_clear(hosts);
    hostlist_randomize(hosts);

    hostlist_destroy(hosts);
}

TEST_F(Hostlist, testCycle)
{
    hostlist_t hosts = hostlist_create();
    lcb_host_t *curhost;


    // Empty list
    ASSERT_EQ(NULL, hostlist_shift_next(hosts, 0));
    ASSERT_EQ(NULL, hostlist_shift_next(hosts, 1));
    hostlist_destroy(hosts);
    hosts = hostlist_create();

    hostlist_add_stringz(hosts, "1.1.1.1", 8091);
    curhost = hostlist_shift_next(hosts, 0);
    ASSERT_TRUE(curhost != NULL);
    ASSERT_TRUE(hostEquals(*curhost, "1.1.1.1", "8091"));

    curhost = hostlist_shift_next(hosts, 0);
    ASSERT_TRUE(hostlist_shift_next(hosts, 0) == NULL);
    ASSERT_TRUE(hostlist_shift_next(hosts, 0) == NULL);
    ASSERT_TRUE(hosts->ix == 1);

    curhost = hostlist_shift_next(hosts, 1);
    ASSERT_TRUE(curhost != NULL);
    ASSERT_TRUE(hostEquals(*curhost, "1.1.1.1", "8091"));

    hostlist_add_stringz(hosts, "2.2.2.2", 8091);
    curhost = hostlist_shift_next(hosts, 0);
    ASSERT_TRUE(hostEquals(*curhost, "2.2.2.2", "8091"));
    ASSERT_TRUE(hostlist_shift_next(hosts, 0) == NULL);

    curhost = hostlist_shift_next(hosts, 1);
    ASSERT_TRUE(hostEquals(*curhost, "1.1.1.1", "8091"));
    curhost = hostlist_shift_next(hosts, 0);
    ASSERT_TRUE(hostEquals(*curhost, "2.2.2.2", "8091"));

    hostlist_clear(hosts);
    ASSERT_TRUE(hostlist_shift_next(hosts, 1) == NULL);
    hostlist_destroy(hosts);
}
