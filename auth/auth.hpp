#include <string>
#include <vector>

namespace Just {
	class api {
	public:

		void init(); // initialize auth

		bool intialized = false;

		std::string version, applicationId;
		api(std::string applicationId, std::string version) : applicationId(applicationId), version(version) {}


		void login(std::string id); // login user using hwid

		std::string get_hwid();

		class user_data {
		public:
			// user data
			std::string hwid;
			std::string expiry;
		};
		user_data user;

		class response_data {
		public:
			// response data
			std::string message;  // show response messages
			std::string key;      // get key
			bool isLogged;        // check if the user is logged in
			bool sucess;         
		};
		response_data response;
	private:
		void update_response(bool sucess, std::string message) {
			api::response.sucess = sucess;
			api::response.message = message;
		};
	};
}