#include <SC_LanguageClient.h>


int main(int argc, char** argv){
  SC_LanguageClient * client = createLanguageClient("sclang");
  
  if(!client)
    return 1;
  
  int returnCode = client -> run(argc,argv);

  destroyLanguageClient(client);
  
  return returnCode;
}
