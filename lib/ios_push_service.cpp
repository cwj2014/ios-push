#include "ios_push_service.h"
#include <errno.h>
#include <chrono>
#include <capn/apn.h>
using namespace std::chrono;

#define CC(__ret__) __check(__ret__)

namespace cyyd {

////////////////////////////////////////////////////////////////////////////////

void __check(uint8_t ret) {
  if (ret != APN_SUCCESS) {
    // failed
	char *err = apn_error_string(errno);
    std::string err_msg = err;
    free(err);
    throw std::runtime_error(err_msg.c_str());
  }
}

void __apn_logging(apn_log_levels level, const char * const message, uint32_t len) {
    const char *prefix = NULL;
    switch(level) {
        case APN_LOG_LEVEL_INFO:
            prefix = "inf";
            break;
        case APN_LOG_LEVEL_ERROR:
            prefix = "err";
            break;
        case APN_LOG_LEVEL_DEBUG:
            prefix = "dbg";
            break;
    }

    printf("======> [apn][%s]: %s\n", prefix, message);
}

void __apn_invalid_token(const char * const token, uint32_t index) {
    printf("======> Invalid token: %s (index: %d)\n", token, index);
}

////////////////////////////////////////////////////////////////////////////////

PushAction::PushAction(std::string const& token,
                       std::string const& text, int badge,
                       std::string const& sound, int exp_secs)
    : _payload_ctx(NULL) , _tokens(NULL), token_buf(NULL){
  _token_str.push_back(token);
  _text = text;
  _badge = badge;
  _sound = sound;
  _exp_secs = exp_secs;
}

PushAction::PushAction(std::vector<std::string> const& tokens, std::string const& text,
              int badge, std::string const& sound, int exp_secs)
          : _payload_ctx(NULL) , _tokens(NULL), token_buf(NULL){
	   _token_str.insert(_token_str.begin(),tokens.begin(), tokens.end());
	  _text = text;
	  _badge = badge;
	  _sound = sound;
	  _exp_secs = exp_secs;
}

PushAction::~PushAction() { close(); }

bool PushAction::initAction(){
	try {
		_payload_ctx = apn_payload_init();
		if(_payload_ctx == NULL){
			return false;
		}
		CC(apn_payload_set_badge(static_cast<apn_payload_t*>(_payload_ctx), _badge)); // Icon badge
		CC(apn_payload_set_body(static_cast<apn_payload_t*>(_payload_ctx), _text.c_str()));  // Notification text
		auto now = time_point_cast<std::chrono::seconds>(system_clock::now());
		apn_payload_set_expiry(static_cast<apn_payload_t*>(_payload_ctx), now.time_since_epoch().count() + _exp_secs); // Expires
		apn_payload_set_sound(static_cast<apn_payload_t*>(_payload_ctx), _sound.c_str());
		apn_payload_set_priority(static_cast<apn_payload_t*>(_payload_ctx), APN_NOTIFICATION_PRIORITY_HIGH);  // Notification priority
		int n = _token_str.size();
		if(n > 0){
			_tokens = apn_array_init(n, NULL, NULL);
			token_buf = (char**)malloc(n*sizeof(char*));
			int index = 0;
			for(auto v: _token_str){
				char * buf = (char*)malloc(sizeof(char)*v.length());
				memcpy(buf, v.c_str(), sizeof(char)*v.length());
				apn_array_insert(static_cast<apn_array_t*>(_tokens),buf);
				token_buf[index++] = buf;
			}
		}

   }
   catch (std::exception& e) {
	  close();
	  return false;
   }
   return true;
}
void PushAction::close() {
  if (_payload_ctx) {
    apn_payload_free(static_cast<apn_payload_t*>(_payload_ctx));
    _payload_ctx = NULL;
  }
  if (_tokens) {
	  apn_array_free(static_cast<apn_array_t*>(_tokens));
	  _tokens = NULL;
  }
  if(token_buf){
	  int n = _token_str.size();
	  for(int i=0; i<n; i++){
		  free(token_buf[i]);
		  token_buf[i] = NULL;
	  }
	  free(token_buf);
  }
}

////////////////////////////////////////////////////////////////////////////////

PushManager::PushManager(std::string const& cert_path,
                         std::string const& keyword)
    : _ctx(NULL) , _cert_path(cert_path), _keyword(keyword){
}

bool PushManager::initPushManager(){
	try {
		CC(apn_library_init());
		_ctx = apn_init();
		if(_ctx == NULL){
			throw new std::runtime_error("apn_init failed");
		}

		CC(apn_set_pkcs12_file(static_cast<apn_ctx_t*>(_ctx), _cert_path.c_str(), _keyword.c_str()));
		apn_set_mode(static_cast<apn_ctx_t*>(_ctx),  APN_MODE_SANDBOX); //APN_MODE_PRODUCTION or APN_MODE_SANDBOX
		apn_set_log_callback(static_cast<apn_ctx_t*>(_ctx), __apn_logging);
		apn_set_behavior(static_cast<apn_ctx_t*>(_ctx), APN_OPTION_RECONNECT);
		apn_set_invalid_token_callback(static_cast<apn_ctx_t*>(_ctx), __apn_invalid_token);
		CC(apn_connect(static_cast<apn_ctx_t*>(_ctx)));
		return true;
    }
    catch (std::exception& e) {
	  close();
	  return false;
    }
}

PushManager::~PushManager() { close(); }

void PushManager::close() {
  if (_ctx) {
    apn_free(static_cast<apn_ctx_t*>(_ctx));
    _ctx = NULL;
  }
  apn_library_free();
}

void PushManager::send(std::unique_ptr<PushAction>& action) {
  if(action == nullptr){
	  printf("Notification failed, PushAction is null\n");
	  return;
  }
  apn_array_t *invalid_tokens = NULL;
  if(APN_SUCCESS != apn_send(static_cast<apn_ctx_t*>(_ctx), static_cast<apn_payload_t*>(action->payload_ctx()), static_cast<apn_array_t*>(action->tokenArray()),&invalid_tokens)){
	  char *err = apn_error_string(errno);
	  std::string err_msg = err;
	  free(err);
	  printf("Notification failed, errno:%d,err_info:%s\n", errno, err_msg.c_str());
  }else{
	  printf("Notification was successfully sent to %u device(s)\n",
	                apn_array_count(static_cast<apn_array_t*>(action->tokenArray())) - ((invalid_tokens) ? apn_array_count(invalid_tokens) : 0));
	  if (invalid_tokens) {
		 printf("Invalid tokens:\n");
		 uint32_t i = 0;
		 for (; i < apn_array_count(invalid_tokens); i++) {
			 printf("    %u. %s\n", i, (char*)apn_array_item_at_index(invalid_tokens, i));
		 }
		 apn_array_free(invalid_tokens);
	  }
  }
}

std::unique_ptr<PushAction> PushManager::create_action(std::string const& token,
                                       std::string const& text, int badge,
                                       std::string const& sound, int exp_secs) {
   auto action = std::make_unique<PushAction>(token, text, badge, sound, exp_secs);
   if(action->initAction()){
      return action;
   }
   return nullptr;
}
std::unique_ptr<PushAction> PushManager::create_action(std::vector<std::string> const& tokens, std::string const& text,
                              int badge, std::string const& sound,
                              int exp_secs){
	auto action = std::make_unique<PushAction>(tokens, text, badge, sound, exp_secs);
    if(action->initAction()){
	  return action;
    }
    return nullptr;
}

}  // namespace apns
