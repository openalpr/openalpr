
#include <unistd.h>
#include <sstream>

#include "tclap/CmdLine.h"
#include "beanstalk.hpp"
#include "alpr.h"
#include "openalpr/simpleini/simpleini.h"
#include "openalpr/cjson.h"
#include "support/tinythread.h"
#include "videobuffer.h"
#include "uuid.h"
#include <curl/curl.h>


// prototypes
void streamRecognitionThread(void* arg);
bool writeToQueue(std::string jsonResult);
bool uploadPost(std::string url, std::string data);
void dataUploadThread(void* arg);

// Constants
const std::string DEFAULT_LOG_FILE_PATH="/var/log/openalpr.log";
const std::string WTS_CONFIG_FILE_PATH="/etc/openalpr/wts.conf";

const std::string BEANSTALK_QUEUE_HOST="127.0.0.1";
const int BEANSTALK_PORT=11300;
const std::string BEANSTALK_TUBE_NAME="alprd";

struct CaptureThreadData
{
  std::string stream_url;
  std::string site_id;
  int camera_id;
  
  std::string config_file;
  std::string country_code;
  std::string output_image_folder;
};

struct UploadThreadData
{
  std::string upload_url;
};

bool daemon_active;

int main( int argc, const char** argv )
{
  daemon_active = true;

  bool noDaemon = false;
  std::string logFile;
  int topn;
  
  std::string configFile;
  std::string country;

  TCLAP::CmdLine cmd("OpenAlpr Daemon", ' ', Alpr::getVersion());

  TCLAP::ValueArg<std::string> countryCodeArg("c","country","Country code to identify (either us for USA or eu for Europe).  Default=us",false, "us" ,"country_code");
  TCLAP::ValueArg<std::string> configFileArg("","config","Path to the openalpr.conf file.",false, "" ,"config_file");
  TCLAP::ValueArg<int> topNArg("n","topn","Max number of possible plate numbers to return.  Default=10",false, 10 ,"topN");
  TCLAP::ValueArg<std::string> logFileArg("l","log","Log file to write to.  Default=" + DEFAULT_LOG_FILE_PATH,false, DEFAULT_LOG_FILE_PATH ,"topN");

  TCLAP::SwitchArg daemonOffSwitch("f","foreground","Set this flag for debugging.  Disables forking the process as a daemon and runs in the foreground.  Default=off", cmd, false);

  try
  {
    
    cmd.add( topNArg );
    cmd.add( configFileArg );
    cmd.add( logFileArg );

    
    if (cmd.parse( argc, argv ) == false)
    {
      // Error occured while parsing.  Exit now.
      return 1;
    }

    country = countryCodeArg.getValue();
    configFile = configFileArg.getValue();
    logFile = logFileArg.getValue();
    topn = topNArg.getValue();
    noDaemon = daemonOffSwitch.getValue();
  }
  catch (TCLAP::ArgException &e)    // catch any exceptions
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }
  
  if (noDaemon == false)
  {
    // Fork off into a separate daemon
    daemon(0, 0);
    
    // Redirect std out to log file
    
    std::ofstream out(logFile.c_str());
    std::cout.rdbuf(out.rdbuf());
    std::cerr.rdbuf(out.rdbuf());
    
    std::cout << "Running OpenALPR daemon in daemon mode." << std::endl;
  }
  else
  {
    std::cout << "Running OpenALPR daemon in the foreground" << std::endl;
  }
  
  CSimpleIniA ini;
  ini.SetMultiKey();
  
  ini.LoadFile(WTS_CONFIG_FILE_PATH.c_str());
  
  std::vector<std::string> stream_urls;
  
  
  CSimpleIniA::TNamesDepend values;
  ini.GetAllValues("daemon", "stream", values);

  // sort the values into the original load order
  values.sort(CSimpleIniA::Entry::LoadOrder());

  // output all of the items
  CSimpleIniA::TNamesDepend::const_iterator i;
  for (i = values.begin(); i != values.end(); ++i) { 
      stream_urls.push_back(i->pItem);
  }
  
  
  if (stream_urls.size() == 0)
  {
    std::cout << "No video streams defined in the configuration" << std::endl;
    return 1;
  }
  
  std::string imageFolder = ini.GetValue("daemon", "image_folder", "/tmp/");
  std::string upload_url = ini.GetValue("daemon", "upload_address", "");
  std::string site_id = ini.GetValue("daemon", "site_id", "");
  
  std::cout << "Using: " << imageFolder << " for storing valid plate images" << std::endl;
  
  for (int i = 0; i < stream_urls.size(); i++)
  {
    CaptureThreadData* tdata = new CaptureThreadData();
    tdata->stream_url = stream_urls[i];
    tdata->camera_id = i + 1;
    tdata->config_file = configFile;
    tdata->output_image_folder = imageFolder;
    tdata->country_code = country;
    tdata->site_id = site_id;
    
    tthread::thread* t = new tthread::thread(streamRecognitionThread, (void*) tdata);
  }

  // Kick off the data upload thread
  UploadThreadData* udata = new UploadThreadData();
  udata->upload_url = upload_url;
  tthread::thread* t = new tthread::thread(dataUploadThread, (void*) udata );

  while (daemon_active)
  {
    usleep(30000);
  }

}


void streamRecognitionThread(void* arg)
{
  CaptureThreadData* tdata = (CaptureThreadData*) arg;
  
  std::cout << "country: " << tdata->country_code << " -- config file: " << tdata->config_file << std::endl;
  std::cout << "Stream " << tdata->camera_id << ": " << tdata->stream_url << std::endl;
  
  Alpr alpr(tdata->country_code, tdata->config_file);
  
  
  std::cout << "asdf" << std::endl;
  
  
  int framenum = 0;
  
  VideoBuffer videoBuffer;
  
  videoBuffer.connect(tdata->stream_url, 5);
  
  cv::Mat latestFrame;
  
  std::vector<uchar> buffer;
  
  std::cout << "Daemon active: " << daemon_active << std::endl;
  
  while (daemon_active)
  {
    int response = videoBuffer.getLatestFrame(&latestFrame);
    
    if (response != -1)
    {
      cv::imencode(".bmp", latestFrame, buffer );
      std::vector<AlprResult> results = alpr.recognize(buffer);
      
      if (results.size() > 0)
      {
	// Create a UUID for the image
	std::string uuid = newUUID();
	
	// Save the image to disk (using the UUID)
	std::stringstream ss;
	ss << tdata->output_image_folder << "/" << uuid << ".jpg";
	
	cv::imwrite(ss.str(), latestFrame);
	
	// Update the JSON content to include UUID and camera ID
	cJSON *root;
  
	root=cJSON_CreateObject();	
  
	std::string json = alpr.toJson(results);
	
	cJSON *array = cJSON_Parse(json.c_str());
	cJSON_AddStringToObject(root,	"uuid",		uuid.c_str());
	cJSON_AddNumberToObject(root,	"camera_id",	tdata->camera_id);
	cJSON_AddStringToObject(root, "site_id", 	tdata->site_id.c_str());
	cJSON_AddItemToObject(root, 	"results", 	array);

	char *out;
	out=cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	
	std::string response(out);
	
	free(out);
	
	// Push the results to the Beanstalk queue
	writeToQueue(response);
      }
    }
    
    usleep(10000);
  }
  
  
  videoBuffer.disconnect();
  
  std::cout << "Video processing ended" << std::endl;
  
  delete tdata;
}


bool writeToQueue(std::string jsonResult)
{
  
    Beanstalk::Client client(BEANSTALK_QUEUE_HOST, BEANSTALK_PORT);
    client.use(BEANSTALK_TUBE_NAME);

    int id = client.put(jsonResult);
    
    if (id <= 0)
      return false;
    
    std::cout << "put job id: " << id << std::endl;

    return true;
}



void dataUploadThread(void* arg)
{
  
  /* In windows, this will init the winsock stuff */ 
  curl_global_init(CURL_GLOBAL_ALL);
  
  
  UploadThreadData* udata = (UploadThreadData*) arg;
  
  Beanstalk::Client client(BEANSTALK_QUEUE_HOST, BEANSTALK_PORT);

  
  client.watch(BEANSTALK_TUBE_NAME);
  
  while(daemon_active)
  {
    Beanstalk::Job job;
    
    client.reserve(job);
    
    if (job.id() > 0)
    {
      if (uploadPost(udata->upload_url, job.body()))
      {
	client.del(job.id());
	std::cout << "Job: " << job.id() << " successfully uploaded" << std::endl;
      }
      else
      {
	client.release(job);
	std::cout << "Job: " << job.id() << " failed to upload.  Will retry." << std::endl;
      }
    }
    
    usleep(10000);
  }
  
  curl_global_cleanup();
}


bool uploadPost(std::string url, std::string data)
{
  bool success = true;
  CURL *curl;
  CURLcode res;
 
 
  /* get a curl handle */ 
  curl = curl_easy_init();
  if(curl) {
    /* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. */ 
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    /* Now specify the POST data */ 
    //char* escaped_data = curl_easy_escape(curl, data.c_str(), data.length());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    //curl_free(escaped_data);
 
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
    {
      success = false;
    }
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
  
  return success;

  
}