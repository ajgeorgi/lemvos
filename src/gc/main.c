#include <stdio.h>
#include <string.h>
#include <locale.h>

/* Main for GC  29.09.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "vertex.h"
#include "loader.h"
#include "modelcreator.h"
#include "configuration.h"
#include "modelstore.h"
#include "shapes.h"


// Models
#include "creator_cube.h"
#include "creator_lemvos.h"
#include "creator_test.h"


int init()
{
    initShapes(0);
    
    modelcreatorAddModel("CUBE",createCubeModel,NULL);
    modelcreatorAddModel(LEMVOS_MODEL_NAME,createLemvos,initLemvos);
    modelcreatorAddModel("test",createTest,initTest);
    
    setlocale(LC_ALL,"en_US");
    // setlocale(LC_NUMERIC,"de_DE");
    struct lconv *locale = localeconv () ;
    
    LOG("Locale: thousands_sep=\"%s\", grouping=\"%s\"\n",locale->thousands_sep,locale->grouping);

    // locale->thousands_sep = ",";
    // locale->grouping = "30";

    return 0;
}

int main(int argc, char **argv)
{
    char fileName[128];
    const char* modelName = "test";
    char configBuffer[256];
    
    if (argc > 1)
    {        
        modelName = argv[1];
    }

    // Setup memory checking here! Needs first initialization.
    initCommen("gc",NULL);
    
    initModelstore();
 
    Model* model = NULL;

    const char*configFileName =  loaderGetConfigNameFromModel(modelName, configBuffer, sizeof(configBuffer));
            
    loaderLoadConfig(configFileName);

    // We need the configuration for init
    init();

    configDump(stderr);

    model = modelcreatorCreate(modelName);             

    int err = 0;

    if (model)
    {
        strncpy(fileName,modelName,sizeof(fileName));
        commenStringCat(fileName,".model",sizeof(fileName));
    
        err = loaderSaveModel(model,fileName);
        if (0 <= err)
        {
            LOG("Model \"%s\" has been created with %u vertices\n",modelName,modelstoreGetVertexCount());
        }
        else
        {
            ERROR("Failure while creating \"%s\"\n",modelName);
            err = -1;
        }
    }
    else
    {
        ERROR("Can not create \"%s\"\n",modelName);
        err = -1;        
    }
    
    modelstoreClear();
    
    finalizeCommen();

    return err;
}
