////////////////////////////////////////////////////////////////////////////////
//
//  Designed by caiyu, 2018-04-18
//
//  Warning: This source code is based on libcapn.
//
//  please visit: https://github.com/cwj2014/ios-push.git
//  for more information.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __ios_push_service__
#define __ios_push_service__

#include <string>
#include <stdexcept>
#include <vector>
#include <memory>


namespace cyyd {

////////////////////////////////////////////////////////////////////////////////

class PushManager;
/**
 *IOS推送实体
 */
class PushAction {
public:
  /**
  构造函数
  @token, 苹果token
  @text, 推送内容
  @badge, app外面的数字角标
  @sound, 声音
  @exp_secs, 有效时间
  */
  PushAction(std::string const& token, std::string const& text,
             int badge, std::string const& sound = "default", int exp_secs = 60 * 60);
  /**
  @tokens, 苹果token集合
  @text, 推送内容
  @badge, app外面的数字角标
  @sound, 声音
  @exp_secs, 有效时间
  */
  PushAction(std::vector<std::string> const& tokens, std::string const& text,
               int badge, std::string const& sound = "default", int exp_secs = 60 * 60);
  /**
  析构函数
  */
  ~PushAction();
public:
  /**
  初始化推送实体
  */
  bool initAction();

  friend class PushManager;
private:
  void* payload_ctx() { return _payload_ctx; }
  void* tokenArray() {return _tokens;}
  void close();
  std::string const& text() const { return _text; }
  int badge() const { return _badge; }
  std::string const& sound() const { return _sound; }
  int exp_secs() const { return _exp_secs; }
private:
  void * _payload_ctx;
  void *_tokens;
  std::vector<std::string> _token_str;
  std::string _text;
  int _badge;
  std::string _sound;
  int _exp_secs;
  char** token_buf;
};

////////////////////////////////////////////////////////////////////////////////
/**
 * IOS推送执行者
 */
class PushManager {
public:
   /**
   构造函数
   @cert_path, 开发证书
   @keyword, 密码
   */
   PushManager(std::string const& cert_path, std::string const& keyword);
   /**
   析构函数
   */
   ~PushManager();
   /**
   初始化IOS推送执行者
   */
   bool initPushManager();
   /**
   关闭
   */
   void close();
   /**
   推送函数
   */
   void send(std::unique_ptr<PushAction>& action);
   /**
   创建推送实体
   @token, 苹果token
   @text, 推送内容
   @badge, app外面的数字角标
   @sound, 声音
   @exp_secs, 有效时间
   */
   std::unique_ptr<PushAction> create_action(std::string const& token, std::string const& text,
                            int badge, std::string const& sound,
                            int exp_secs = 60 * 60);
   /**
   创建推送实体
   @tokens, 苹果token集合
   @text, 推送内容
   @badge, app外面的数字角标
   @sound, 声音
   @exp_secs, 有效时间
   */
   std::unique_ptr<PushAction> create_action(std::vector<std::string> const& tokens, std::string const& text,
                              int badge, std::string const& sound,
                              int exp_secs = 60 * 60);
private:
  void* _ctx;
  std::string _cert_path;
  std::string _keyword;
};

}  // namespace cyyd

#endif /* defined(__ios_push_service__) */
