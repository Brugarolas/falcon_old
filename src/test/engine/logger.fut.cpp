/*****************************************************************************
  FALCON2 - The Falcon Programming Language
  FILE: logger.fut.cpp

  Test default logger
  -------------------------------------------------------------------
  Author: Giancarlo Niccolai
  Begin : Thu, 28 Feb 2019 22:02:59 +0000
  Touch : Sat, 02 Mar 2019 01:33:45 +0000

  -------------------------------------------------------------------
  (C) Copyright 2019 The Falcon Programming Language
  Released under Apache 2.0 License.
******************************************************************************/

#include <falcon/fut/fut.h>
#include <falcon/logger.h>

#include <iostream>

#include <future>

class TestListener: public Falcon::LogSystem::Listener {
public:
	std::promise<Falcon::LogSystem::Message> m_msgPromise;

protected:
    virtual void onMessage( const Falcon::LogSystem::Message& msg ) override{
    	m_msgPromise.set_value(msg);
    }
};


class LoggerTest: public Falcon::testing::TestCase
{
public:
   void SetUp() {
      m_catcher = std::make_shared<TestListener>();
      LOG.addListener(m_catcher);
      m_caught = m_catcher->m_msgPromise.get_future();
    }

   void TearDown() {
	   // Detach the catcher stream, or it will still be there
	   // the next time we use this fixture!
	   m_catcher->detach();
   }

   using FutureMessage = std::future<Falcon::LogSystem::Message>;

   bool waitResult(FutureMessage& msg) {
	   std::chrono::seconds span (5);
	   if( msg.wait_for(span) == std::future_status::timeout) {
	   		FAIL("Message not received by logger");
	   		return false;
	   }
	   return true;
   }

   std::shared_ptr<TestListener> m_catcher;
   FutureMessage m_caught;

};


TEST_F(LoggerTest, Smoke)
{
   std::ostringstream tempStream;
   LOG.defaultListener()->writeOn(&tempStream);
   LOG << "Hello World";
   waitResult(m_caught);
   EXPECT_NE(tempStream.str().find("Hello World"), std::string::npos);
}


TEST_F(LoggerTest, Category)
{
   std::ostringstream tempStream;
   LOG.defaultListener()->writeOn(&tempStream);
   LOG.setCategory("The Category");
   LOG << "Hello World";
   waitResult(m_caught);
   EXPECT_NE(tempStream.str().find("The Category"), std::string::npos);
}

FALCON_TEST_MAIN

/* end of logger.fut.cpp */
