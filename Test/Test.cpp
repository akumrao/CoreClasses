/*****************************************
 * Test code for
 Common Core reusable classes for String, Thread, Token Generation & Expiration, UUID, CoreID, Loggin, Profiling
  
 */

#include "../Core/String.hxx"
#include "../Core/GenToken.hxx"
#include "../Core/SecurityToken.hxx"

#include "../Core/gl_log.h"

#define STR_DEBUG(_L)   __FILE__, __LINE__, "STREAM", GL_LEVEL_DEBUG + _L
#define STR_ERROR       __FILE__, __LINE__, "STREAM", GL_LEVEL_ERROR
#define STR_TRACE       __FILE__, __LINE__, "STREAM", GL_LEVEL_TRACE

#include <uuid/uuid.h>  //apt-get install uuid uuid-dev

int main(int arc, char** argv) {


    gut_init_logger(argv[0], NULL, "/tmp/test.log");

    String permissions("VT");
    String deviceUid("ID");
    String appkey = "appkey";

    uuid_t uuid;
    char coreID[64];

    uuid_generate(uuid);
    uuid_unparse_lower(uuid, coreID);
    //char coreID[100];
    //uuid_unparse(uuid, coreID);
    deviceUid.assign(coreID);



    gut_log(STR_ERROR, " perm %s, dev %s appkey %s", permissions.c_str(), deviceUid.c_str(), appkey.c_str());


    timeval now;
    gettimeofday(&now, NULL);

    String token = SecToken::createSecurityToken(deviceUid, permissions, appkey);

    gut_log(STR_ERROR, "token %s", token.c_str());

    if (token.empty()) {
        gut_log(STR_ERROR, "Unable to get token for dev (%s)", deviceUid.c_str());
    }

    // Validate if Token is expired or Not. Uncomment the following line to test if token is expered after 35 secs
    //  sleep( 35);
    String perm;
    String msg;
    uint32_t statusCode;

    MS_SecurityToken obj(token);
    obj.validate(appkey, msg, perm, statusCode, false);

    gut_log(STR_ERROR, "key validate msg %s, perm %s code %u", msg.c_str(), perm.c_str(), statusCode);




    return 0;
}