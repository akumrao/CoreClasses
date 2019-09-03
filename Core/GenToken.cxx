/*
 * ------------------------------------------------------------------
 * July 2013, Arvind Umrao<akumrao@yahoo.com>
 *
 * Copyright (c) 2019  Arvind Umrao
 * All rights reserved.
 * H1 security token = Device Id:userpermissions:ptz Priority:Live QOS:Archive QOS:H1
 * Generate security token

 * ------------------------------------------------------------------
 */

#include <errno.h>
#include <stdio.h>

#include <string>
#include "GenToken.hxx"
#include "Encoders.hxx"
#include "gl_log.h"
#include <sys/time.h>
#include <vector>
#include <iostream>

//#ifdef H1SECURITY_CODE

String SecToken::createSecurityToken(
        String& uid,
        String& permissions,
        const String& key,
        uint8_t ptz,
        uint8_t qosl,
        uint8_t qosa)
{

    String token;
    String h1;
    h1 = calculateH1(uid, permissions, ptz, qosl, qosa, key);

    token.assignf("%s^%s^%d^%d^%d^%s", uid.c_str(), permissions.c_str(), ptz, qosl, qosa, h1.c_str());

    H1_SecurityToken obj(token);
    obj.parse();
    String H1 = obj.getH1();

    timeval now;
    gettimeofday(&now, NULL);

    uint32_t expir = now.tv_sec + 5 * 60;
    String h2;
    h2 = calculateH2(H1, expir);

    token.assignf("%s^%s^%d^%d^%d^%u^%s", obj.getCameraUID().c_str(), obj.getPermissions().c_str(), obj.getPTZPriority(), obj.getQosl(), obj.getQosa(), expir, h2.c_str());

    return token;
}


String SecToken::calculateH1(
        String& uid,
        String& permissions,
        uint8_t ptz,
        uint8_t qosl,
        uint8_t qosa,
        const String& key)
{
    //calculate H1 
    uint8_t digest[SHA1_DIGESTSIZE];
    DigestSHA1 sha1;

    sha1.init();
    String work;
    work.appendf("%s%s%s%d%d%d", key.c_str(), uid.c_str(), permissions.c_str(), ptz, qosl, qosa);
    sha1.update(work.c_str(), work.length());
    sha1.final(digest);

    String temp, temp2;
    temp.assign((const char*) digest, sizeof (digest));
    HexEncoder::encode(temp2, temp);
    return temp2;
}

GenToken::GenToken()
{
}

GenToken::~GenToken()
{
}

/*
 * Obsolete function, will be deleted very soon
 * 
 
String GenToken::getToken(String &deviceUid)
{
    String permissions("VT");
    
    std::string emsg;
    emsg.clear();
    CoolUMSConfigParam* key = CoolUMSConfigParam::load(emsg, "AppSecurityKey");
    if (!emsg.empty())
    {
        gut_log(STR_ERROR, "Unable to get security key (%s)", emsg.c_str());
        return "";
    }

    String appkey;
    if (key != NULL)
    {
        appkey = key->getString().c_str();
        delete key;
    }

    timeval now;
    gettimeofday(&now, NULL);

    String token = SecToken::createSecurityToken(deviceUid, permissions, appkey,
            now.tv_sec +  5*60, 0, 0, 0);
    
    if (token.empty())
    {
        gut_log(STR_ERROR, "Unable to get token for dev (%s)", deviceUid.c_str());
    }
   
    //String perm;
    //String msg;
    //uint32_t statusCode;
   
    // MS_SecurityToken obj(token);
    // obj.validate(appkey, msg, perm, statusCode, false );
    //  validate(token, msg, perm, statusCode, false );
    //gut_log(STR_ERROR, "key validate msg %s, perm %s code %u", msg.c_str(), perm.c_str(), statusCode);
    
    return token;
}
*/
//#endif

