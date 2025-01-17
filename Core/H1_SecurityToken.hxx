/*
 * ------------------------------------------------------------------
 * Oct 2013, Arvind Umrao<akumrao@yahoo.com>
 *
 * Copyright (c) 2013 byArvind Umrao.
 * All rights reserved.
 * 
 * Generate security token
 * H1 security token = Camera Id:userpermissions:ptz Priority:Live QOS:Archive QOS:H1
 * H2 =  Sha-1(H1 + expiry time) From H1 security token take H1 and rehash it by appending expiry time
 * Final security token (with H2) = Camera Id:userpermissions:ptz Priority:Live QOS:Archive QOS:expiry time:H2
 * ------------------------------------------------------------------
 */

#ifndef H1_SECURITYTOKEN
#define	H1_SECURITYTOKEN

#include "String.hxx"
#include "Digest.hxx"

class H1_SecurityToken
{
public:

    H1_SecurityToken(const String& token)
    {
        clear();
        m_token.assign(token);
    }

    H1_SecurityToken()
    {
        clear();
    }

    ~H1_SecurityToken()
    {
        clear();
    }

    void clear()
    {
        m_token.clear();
        m_cameraUID.clear();
        m_permissions.clear();
        m_h1.clear();
        m_qosl = 0;
        m_qosa = 0;
        m_ptzPriority = 0;
    }

    enum
    {
        OK = 200, UNAUTHORIZED = 401, AUTHORIZATION_EXPIRED = 402, FORBIDDEN = 403
    };

    const String& getToken() const
    {
        return m_token;
    }

    uint8_t getQosl() const
    {
        return m_qosl;
    }

    uint8_t getQosa() const
    {
        return m_qosa;
    }

    uint8_t getPTZPriority() const
    {
        return m_ptzPriority;
    }

    const String getPermissions() const
    {
        return m_permissions;
    }

    const String& getCameraUID() const
    {
        return m_cameraUID;
    }

    const String& getH1() const
    {
        return m_h1;
    }

    void init(const String& token)
    {
        m_token.assign(token);
    }

    static const char SECURITY_TOKEN_SEPARATOR = '^';

    static String getSecurityToken(String &tokenH1);

    static String calculateH2(const String& H1, uint32_t expirationTime);

    bool parse();
protected:

    static const int SECURITY_TOKEN_ITEMS = 6;

    enum
    {
        CAMUID = 0, PERM, PTZPRI, QOSL, QOSA, H1
    };

private:
    String m_token;
    String m_cameraUID;
    String m_permissions;
    uint8_t m_qosl;
    uint8_t m_qosa;
    uint8_t m_ptzPriority;
    String m_h1;
};

#endif	/* H1_SECURITYTOKEN */
