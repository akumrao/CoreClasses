/*
 * ------------------------------------------------------------------
 * July 2013, Arvind Umrao<akumrao@yahoo.com>
 *
 * Copyright (c) 2013 by Arvind Umrao.
 * All rights reserved.
 * 
 * Generate security tocken
 * ------------------------------------------------------------------
 */

#ifndef GenToken_HPP
#define	GenToken_HPP


#include "H1_SecurityToken.hxx"

//#ifdef H1SECURITY_CODE

class SecToken : public H1_SecurityToken
{
public:

    static String createSecurityToken(String& uid,
            String& permissions,
            const String& key,
            uint8_t ptz = 0,
            uint8_t qosl = 0,
            uint8_t qosa = 0);

    static String calculateH1(String& uid,
            String& permissions,
            uint8_t ptz,
            uint8_t qosl,
            uint8_t qosa,
            const String& key);
};

class GenToken
{
public:
    GenToken();
  //  static String getToken(String &deviceUid);
    virtual ~GenToken();
private:

};

//#endif

#endif	/* GenToken_HPP */

