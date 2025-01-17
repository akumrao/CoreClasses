/*
 * ------------------------------------------------------------------
 * Oct 2013, Arvind Umrao<akumrao@yahoo.com>
 *
 * Copyright (c) 2013 byArvind Umrao..
 * All rights reserved.
 * 
 * Generate security token
 * H2 =  Sha-1(H1 + expiry time) From H1 security token take H1 and rehash it by appending expiry time
 * Final security token (with H2) = Camera Id:userpermissions:ptz Priority:Live QOS:Archive QOS:expiry time:H2
 * ------------------------------------------------------------------
 */


#include "H1_SecurityToken.hxx"
#include "Encoders.hxx"
#include "gl_log.h"
#include <sys/time.h>
#include <vector>
#include <iostream>

/*
 ** MediaServer security token helper functions
 **
 */
const int H1_SecurityToken::SECURITY_TOKEN_ITEMS;
const char H1_SecurityToken::SECURITY_TOKEN_SEPARATOR;

bool H1_SecurityToken::parse()
{
    // parse m_token based on the following string format
    //	CameraUID^Permissions^PTZ-Priority^QosLive^QosRec^Expiration^H2
    std::vector<String> sdata;
    String::size_type spos = 0, pos = 0;
    if (m_token.empty())
    {
        gut_log(GUT_ERROR, "Security token is missing from the request.");
        return false;
    }
    while (spos < m_token.length())
    {
        pos = m_token.find(SECURITY_TOKEN_SEPARATOR, spos);
        if (pos == String::npos) break;
        sdata.push_back(m_token.substr(spos, pos - spos));
        spos = pos + 1;
    }
    if (spos < m_token.length())
    {
        sdata.push_back(m_token.substr(spos));
    }
    if (sdata.size() < SECURITY_TOKEN_ITEMS)
    {
        return false;
    }

    m_cameraUID = sdata[CAMUID];
    m_permissions = sdata[PERM];
    m_ptzPriority = sdata[PTZPRI].as_int32();
    m_qosl = sdata[QOSL].as_int32();
    m_qosa = sdata[QOSA].as_int32();
    m_h1 = sdata[H1];

    sdata.clear();
    return true;

}

String H1_SecurityToken::calculateH2(
        const String& H1,
        uint32_t expirationTime)
{
    //calculate H1 
    uint8_t digest[SHA1_DIGESTSIZE];
    DigestSHA1 sha1;

    String work;

    //calculate H2 = SHA1(H1, expiration time)
    sha1.init();
    work.clear();
    work.appendf("%s%u", H1.c_str(), expirationTime);
    sha1.update(work.c_str(), work.length());
    sha1.final(digest);

    // hex encode the digest and append to the date string
    String sdigest, comp_h2;
    sdigest.assign((const char*) digest, sizeof (digest));
    HexEncoder::encode(comp_h2, sdigest);

    return comp_h2;
}

String H1_SecurityToken::getSecurityToken(String &tokenH1)
{
    String token;

    H1_SecurityToken obj(tokenH1);
    if (obj.parse())
    {
        String H1 = obj.getH1();

        timeval now;
        gettimeofday(&now, NULL);

        uint32_t expir = now.tv_sec + 5 * 60;
        String h2;
        h2 = calculateH2(H1, expir);
        token.assignf("%s^%s^%d^%d^%d^%u^%s", obj.getCameraUID().c_str(), obj.getPermissions().c_str(), obj.getPTZPriority(), obj.getQosl(), obj.getQosa(), expir, h2.c_str());
    }
    return token;
}

