//============================================================================
// Name        : apns_test.cpp
// Author      : caiyu
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;
#include "ios_push_service.h"
#include <memory>
#include  <stdio.h>

int main() {
	std::shared_ptr<cyyd::PushManager> pushManager = std::make_shared<cyyd::PushManager>("APNs_dev_Ubar_170609.p12","123456");
	if(!pushManager->initPushManager()){
		printf("pushManager init failed\n");
	}

	auto action = pushManager->create_action("3960e83285cb847fdca58a0efdf6622ec8cd6ce7b79bdc05eedfe57b13222bfe","test88888",
				1, "default");
	pushManager->send(action);
	
	return 0;
}
