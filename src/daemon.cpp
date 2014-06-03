
#include <unistd.h>

#include "beanstalk.hpp"
#include "alpr.h"
#include "openalpr/simpleini/simpleini.h"
#include "support/tinythread.h"
#include "videobuffer.h"

// prototypes
void streamRecognitionThread(void* arg);

struct ThreadData
{
  std::string stream_url;  
};

bool daemon_active;

int main( int argc, const char** argv )
{
  daemon_active = true;

  CSimpleIniA ini;
  ini.SetMultiKey();
  
  ini.LoadFile("/etc/openalpr/wts.conf");
  
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
  
  for (int i = 0; i < stream_urls.size(); i++)
  {
    ThreadData* tdata = new ThreadData();
    tdata->stream_url = stream_urls[i];
    tthread::thread* t = new tthread::thread(streamRecognitionThread, (void*) tdata);
  }


  

}


void streamRecognitionThread(void* arg)
{
  ThreadData* tdata = (ThreadData*) arg;
  
  std::cout << "Stream: " << tdata->stream_url << std::endl;
  
  int framenum = 0;
  
  VideoBuffer videoBuffer;
  
  videoBuffer.connect(tdata->stream_url, 5);
  
  cv::Mat latestFrame;
  
  /*
  while (daemon_active)
  {
    int response = videoBuffer.getLatestFrame(&latestFrame);
    
    if (response != -1)
    {
      detectandshow( &alpr, latestFrame, "", outputJson);
    }
    
    //cv::waitKey(10);
  }
  */
  
  videoBuffer.disconnect();
  
  std::cout << "Video processing ended" << std::endl;
  
  delete tdata;
}


bool writeToQueue(AlprResult result)
{
  
    Beanstalk::Client client("127.0.0.1", 11300);
    client.use("test");
    client.watch("test");

    int id = client.put("hello");
    
    if (id <= 0)
      return 1;
    
    std::cout << "put job id: " << id << std::endl;

    Beanstalk::Job job;
    client.reserve(job);
    
    if (job.id() != id)
      return 1;

    std::cout << "reserved job id: "
         << job.id()
         << " with body {" << job.body() << "}"
         << std::endl;

    client.del(job.id());
    std::cout << "deleted job id: " << job.id() << std::endl;
}
