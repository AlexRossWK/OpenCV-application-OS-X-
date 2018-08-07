#include <iostream>
#include <vector>

//#include <stdio.h>
#include <curl/curl.h>

#pragma comment(lib, "rpcrt4.lib")  // UuidCreate - Minimum supported OS Win 2000
#include <windows.h>
#include <iostream>




const char* text2_url = "http://eye-server.woodenshark.com/api/v1";

//JSON
/* Never writes anything, just returns the size presented */
size_t my_dummy_write(char *ptr, size_t size, size_t nmemb, void *userdata)
{
   return size * nmemb;
}

//поменял, теперь сообще будет такое {"camera_id": 3, "message": "какой то текст"}
/*
curl -X POST 'http://api.neuro.cam/video/cameras/notify/' 
-H 'Authorization: 
Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiI3MSIsImlzcyI6ImFpQG5ldXJvLmNhbSJ9.tAXgv9CGmwYc7f3ZRUltCMSQBlrlynhwK9qPnUQiqqA' 
--data-binary '{"camera_id": 2, "message": "some text message"}'
*/
//принимаем тело с статусом и типом сообщения (есть человек или нет)
void SendJSONPeople()
{

	std::string text2_post = "{camera_id: 2, message: some text message}";
	if (false)
	{

		CURL *curl;
		CURLcode res;

		/* In windows, this will init the winsock stuff */
		curl_global_init(CURL_GLOBAL_ALL);

		/* get a curl handle */
		curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, "http://api.neuro.cam/video/cameras/notify/");

			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, text2_post);

			res = curl_easy_perform(curl);
			if (res != CURLE_OK)
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));

			curl_easy_cleanup(curl);
		}
		curl_global_cleanup();
		return;

	}
	struct curl_slist *headersTest = NULL; // init to NULL is important

	headersTest = curl_slist_append(headersTest, "Accept: application/json");
	headersTest = curl_slist_append(headersTest, "Content-Type: application/json");
	headersTest = curl_slist_append(headersTest, "charsets: utf-8");
	headersTest = curl_slist_append(headersTest, "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiI3MSIsImlzcyI6ImFpQG5ldXJvLmNhbSJ9.tAXgv9CGmwYc7f3ZRUltCMSQBlrlynhwK9qPnUQiqqA");




	CURL *curl = curl_easy_init();
	CURLcode res;
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_URL, "http://api.neuro.cam/video/cameras/notify/");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, text2_post);

		/*curl_easy_setopt(curl, CURLOPT_GETFIELDS, text2_post);*/



		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1); //CURLOPT_HTTPGET
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headersTest);
		//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &my_dummy_write);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

		res = curl_easy_perform(curl);



		if (CURLE_OK == res) {
			char *ct;
			/* ask for the content-type */
			res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
			if ((CURLE_OK == res) && ct)
				printf("We received Content-Type: %s\n", ct);
		}

		curl_easy_cleanup(curl);

		//delete res;
	}

	free(headersTest);


}

//принимаем тело с статусом и типом сообщения (есть человек или нет)
void SendJSON(char* text2_post)
{

	if (true)
	{

		CURL *curl;
		CURLcode res;

		/* In windows, this will init the winsock stuff */
		curl_global_init(CURL_GLOBAL_ALL);

		/* get a curl handle */
		curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, "http://eye-server.woodenshark.com/api/v1");

			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, text2_post);

			res = curl_easy_perform(curl);
			if (res != CURLE_OK)
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));

			curl_easy_cleanup(curl);
		}
		curl_global_cleanup();
		return;

	}
    struct curl_slist *headersTest=NULL; // init to NULL is important

    headersTest = curl_slist_append(headersTest, "Accept: application/json");
    headersTest = curl_slist_append(headersTest, "Content-Type: application/json");
    headersTest = curl_slist_append(headersTest, "charsets: utf-8");



    CURL *curl = curl_easy_init();
    CURLcode res;
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, text2_url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, text2_post);

		/*curl_easy_setopt(curl, CURLOPT_GETFIELDS, text2_post);*/



        curl_easy_setopt(curl, CURLOPT_HTTPGET,1); //CURLOPT_HTTPGET
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headersTest);
        //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &my_dummy_write);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,1);

        res = curl_easy_perform(curl);



        if(CURLE_OK == res) {
            char *ct;
            /* ask for the content-type */
            res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
            if((CURLE_OK == res) && ct)
                printf("We received Content-Type: %s\n", ct);
        }

        curl_easy_cleanup( curl );

        //delete res;
    }

    free(headersTest);


}

void SendJSONVideoFile(long size_file,const char* uid)
{
	CURLcode ret;
	CURL *hnd;
	curl_mime *mime1;
	curl_mimepart *part1;
	struct curl_slist *slist1;

	mime1 = NULL;
	slist1 = NULL;
	slist1 = curl_slist_append(slist1, "X-Device-ID: 309c231d555e");

	hnd = curl_easy_init();
	curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, size_file);
	curl_easy_setopt(hnd, CURLOPT_URL, "http://eye-server.woodenshark.com/api/v1/save_video");
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	mime1 = curl_mime_init(hnd);
	part1 = curl_mime_addpart(mime1);
	curl_mime_data(part1, /*"8a665889-b7a2-d9cc-3d40-810d37265fe1"*/uid, CURL_ZERO_TERMINATED);
	curl_mime_name(part1, "log_record");
	part1 = curl_mime_addpart(mime1);
	curl_mime_filedata(part1, "arhive/out_camera0.avi");
	curl_mime_name(part1, "video_file");
	curl_easy_setopt(hnd, CURLOPT_MIMEPOST, mime1);
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.58.0");
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

	CURLcode res;
	ret = curl_easy_perform(hnd);
	if (CURLE_OK == res) {
		char *ct;
		res = curl_easy_getinfo(hnd, CURLINFO_CONTENT_TYPE, &ct);
		if ((CURLE_OK == res) && ct)
			printf("\nWe received Content-Type: %s\n", ct);
	}

	curl_easy_cleanup(hnd);
	hnd = NULL;
	curl_mime_free(mime1);
	mime1 = NULL;
	curl_slist_free_all(slist1);
	slist1 = NULL;
}

long GetFileSize(std::string filename)
{
	struct stat stat_buf;
	int rc = stat(filename.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

int main_11221(int argc, char * argv[])
{
	
	long size_file = GetFileSize("arhive/out_camera0.avi");
	std::cout << size_file << std::endl;
	
	UUID uuid;
	UuidCreate(&uuid);
	char *str_uid;
	UuidToStringA(&uuid, (RPC_CSTR*)&str_uid);
	std::cout << str_uid << std::endl;
	
	//std::string string_text= std::string(str_uid);
	
	SendJSONVideoFile(size_file, str_uid);

	RpcStringFreeA((RPC_CSTR*)&str_uid);

	return 1;
	SendJSONPeople();

	//SendJSON("<methodCall><methodName>save_log</methodName><params><param><value><string>01010</string></value></param><param><value><int>1</int></value></param><param><value><int>2</int></value></param><param><value><string>2018-02-23</string></value></param></params></methodCall>");
	return 1;

}
