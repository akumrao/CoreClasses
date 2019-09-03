/*
 * Copyright (c) 2011-2013 by Arvind Umrao.
 * 
 */

#include "SecurityToken.hxx"
#include "Encoders.hxx"
#include "gl_log.h"
#include <sys/time.h>
#include <vector>
#include <iostream>

/*
 *  create a security token for inter-server communication
 *  param - data to be encrypted in the token
 */
String SecurityToken::createServerToken(
    String& value)
{
    // create a date string
    timeval tv;
    gettimeofday(&tv, NULL);
    String date;
    date.assignf("%x%06x", tv.tv_sec, tv.tv_usec);

    // get salt value for SHA1 from date string
    String salt;
    salt = date.substr(date.length()-10);

    // create a SHA1 digest
    uint8_t digest[SHA1_DIGESTSIZE];
    DigestSHA1 work;

    work.init();
    work.update(salt.c_str(), salt.length());
    work.update(value.c_str(), value.length());
    work.final(digest);

    // hex encode the digest and append to the date string
    for ( uint8_t i=0; i < SHA1_DIGESTSIZE; i++ )
        date.appendf("%02x", digest[i]);

    gut_log(GUT_DEBUG(1), "ServerToken: '%s'", date.c_str());

    // return new date string as the token
    return date;
}





/*
 *  validate a security token for inter-server communication
 *  param - data that was encrypted in the token
 */
bool SecurityToken::isValidServerToken(
    String& token,
    String& value,
    uint32_t timeDiff)
{
    // get the date string from the token
    String date;
    date = token.substr(0, token.length()-(SHA1_DIGESTSIZE*2));

    // get the salt string from the date
    String salt;
    salt = date.substr(date.length()-10);

    gut_log(GUT_DEBUG(1), "token: '%s' salt: '%s'", token.c_str(), salt.c_str());

    // the date is a hex encode time value of the form seconds:microseconds
    String seconds;
    time_t secs;
    // extract seconds from date
    seconds = date.substr(0, date.size()-6);
    sscanf(seconds.c_str(), "%x",  &secs);

    // check time is within 60 seconds of our time
    time_t now = time(NULL);
    time_t diff = now - secs;
    gut_log(GUT_DEBUG(1), "seconds: '%s' secs: %lu now %lu diff %ld", seconds.c_str(), secs, now, diff);
    if ( ::labs(diff) > timeDiff ) {
        gut_log(GUT_ERROR, "Token expired (%ld)", diff);
        return false;
    }

    // create the SHA1 digest
    uint8_t digest[SHA1_DIGESTSIZE];
    DigestSHA1 work;

    work.init();
    work.update(salt.c_str(), salt.length());
    work.update(value.c_str(), value.length());
    work.final(digest);

    // hex encode the digest and append to the date
    for ( uint8_t i=0; i < SHA1_DIGESTSIZE; i++ )
        date.appendf("%02x", digest[i]);

    gut_log(GUT_DEBUG(1), "ServerToken: '%s' token: '%s'", date.c_str(), token.c_str());

    return (date == token);
}





/*
** MediaServer security token helper functions
**
*/
const int MS_SecurityToken::SECURITY_TOKEN_ITEMS;
const char MS_SecurityToken::SECURITY_TOKEN_SEPARATOR;
bool MS_SecurityToken::parse()
{
    // parse m_token based on the following string format
    //  CameraUID^Permissions^PTZ-Priority^QosLive^QosRec^Expiration^H2
    std::vector<String> sdata;
    String::size_type spos = 0, pos = 0;
    if (m_token.empty()) {
        gut_log(GUT_ERROR, "Security token is missing from the request.");
        return false;
    }
    while (spos < m_token.length()) {
        pos = m_token.find(SECURITY_TOKEN_SEPARATOR, spos);
        if (pos == String::npos) break;
        sdata.push_back(m_token.substr(spos, pos - spos));
        spos = pos + 1;
    }
    if (spos < m_token.length())  {
        sdata.push_back(m_token.substr(spos));
    }
    if (sdata.size() < SECURITY_TOKEN_ITEMS)  {
        return false;
    }

    m_cameraUID = sdata[CAMUID];
    m_permissions = sdata[PERM];
    m_ptzPriority = sdata[PTZPRI].as_int32();
    m_qosl = sdata[QOSL].as_int32();
    m_qosa = sdata[QOSA].as_int32();
    m_expirationTime = sdata[EXPIRATION].as_uint32();
    m_h2 = sdata[H2];

    sdata.clear();
    return true;

}





bool MS_SecurityToken::validate(const String& key,
            String& emsg,
             String& perm,
            uint32_t& statusCode,
            bool rtsp)
{
    // validate the format of the data
    statusCode = MS_SecurityToken::OK;
    emsg.assign("200 OK");
    if (!parse()) {
        emsg.assign("401 Unauthorized");
        statusCode = MS_SecurityToken::UNAUTHORIZED;
        return false;
    }
    // validate permissions
    if (!perm.empty()) {
        // rtps checks for at least one permissions from perm to be in m_permissions
        if (rtsp) {
            for (uint8_t i = 0; i < perm.length(); i++) {
                if (m_permissions.find(perm[i]) != String::npos)
                    break;
            }
        } else {
            // http permissions
            for (uint8_t i = 0; i < perm.length(); i++)
                if (m_permissions.find(perm[i]) == String::npos)  {
                    emsg.assign("403 Forbidden");
                    statusCode = MS_SecurityToken::FORBIDDEN;
                    return false;
                }
        }
    }
    // if expiration time is in the near future
    timeval now;
    gettimeofday(&now, NULL);
    if (m_expirationTime < now.tv_sec) {
        emsg.assign("402 Authorization Expired");
        statusCode = MS_SecurityToken::AUTHORIZATION_EXPIRED;
        return false;
    }

    //calculate H2
    String comp_h2;
    comp_h2 = calculateH2(m_cameraUID, m_permissions, m_ptzPriority, m_qosl, m_qosa, key, m_expirationTime);
    bool ret = (comp_h2 == m_h2);
    if (!ret) {
        emsg.assign("401 Unauthorized");
        statusCode = MS_SecurityToken::UNAUTHORIZED;
    }
    else
    {
         if (perm.empty()) 
             perm = m_permissions;
         
    }
    return ret;
}





// stub function. Don't give parameter name for unused parms to avoid unused
// parameter errors.
String
MS_SecurityToken::createSecurityToken (String&,
                                       String&,
                                       const String&,
                                       uint32_t,
                                       uint8_t,
                                       uint8_t,
                                       uint8_t)
{
			return "NOT SUPPORTED";
}





String MS_SecurityToken::calculateH2(
    String& uid,
    String& permissions,
    uint8_t ptz,
    uint8_t qosl,
    uint8_t qosa,
    const String& key,
    uint32_t expirationTime)
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
    temp.assign((const char*)digest, sizeof(digest));
    HexEncoder::encode(temp2, temp);

    //calculate H2 = SHA1(H1, expiration time)
    sha1.init();
    work.clear();
    work.appendf("%s%u", temp2.c_str(), expirationTime);
    sha1.update(work.c_str(), work.length());
    sha1.final(digest);

    // hex encode the digest and append to the date string
    String sdigest, comp_h2;
    sdigest.assign((const char*)digest, sizeof(digest));
    HexEncoder::encode(comp_h2, sdigest);

    return comp_h2;
}
