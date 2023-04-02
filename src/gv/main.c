#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

/* main for GV 19.08.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/


#include "vertex.h"
#include "viewer.h"
#include "controller.h"
#include "loader.h"
#include "modelstore.h"
#include "modelcreator.h"

#include "popup.h"
#include "configuration.h"
#include "projection.h"

#include "exporter.h"

#include "creator_cube.h"
#include "creator_lemvos.h"

static int gv_runlevel = 0;

void sighandler(int signal)
{
    switch(signal)
    {
        case SIGKILL:
        case SIGQUIT:
        case SIGTERM:
        case SIGINT:
        case SIGILL:
            gv_runlevel = 0;
            break;
        case SIGTRAP:
            gv_runlevel = 0;
            break;             
        default:
            gv_runlevel = 0;
    }
}

int init(int argc, char **argv)
{
    /*
    struct sigaction gv_sigaction; 

    memset(&gv_sigaction,0,sizeof(struct sigaction));
    gv_sigaction.sa_handler = sighandler;

    sigaction(SIGKILL, &gv_sigaction, NULL);
    int e1 = sigaction(SIGQUIT, &gv_sigaction, NULL);
    int e2 = sigaction(SIGTERM, &gv_sigaction, NULL);    
    int e3 = sigaction(SIGINT, &gv_sigaction, NULL);
    int e4 = sigaction(SIGILL, &gv_sigaction, NULL);
    sigaction(SIGCHLD, &gv_sigaction, NULL);
    sigaction(SIGTRAP, &gv_sigaction, NULL);        
    
    if ((e1<0) || (e2<0) || (e3<0) || (e4<0))
    {
        fprintf(stderr,"sighandler failed.\n");
        perror("sigaction():");
    }
*/
    // Setup memory checking here! Needs first initialization.
    initCommen("gv",plotterHandleError);

    initModelstore();
    initLoader(plotterHandleError);
    
    viewerInit(argc, argv, plotterHandleError);
    plotterInit();
    
    exporterInit(projectionGetProjection(), plotterHandleError);

    modelcreatorAddModel("CUBE",createCubeModel,NULL);
    modelcreatorAddModel(LEMVOS_MODEL_NAME,createLemvos,initLemvos);
    
    // setlocale(LC_NUMERIC,"de_DE");
    setlocale(LC_ALL,"en_US");
    
    struct lconv *locale = localeconv () ;
    
    LOG("Locale: thousands_sep=\"%s\", grouping=\"%s\"\n",locale->thousands_sep,locale->grouping);
    // locale->thousands_sep = ",";
    // locale->grouping = "30";

    Config *lemvosPath = configCreate(LEMVOS_PATH, MeaUnit_None);
    lemvosPath->string = memory_strdup(argv[0]);
    configStoreConfig(lemvosPath);

    return 0;
}


int main(int argc, char **argv)
{       
    int err = -1;    
    
    init(argc,argv);
    
    Model* model = NULL;
    const char* filename = NULL;
 
    if (argc > 2)
    {
        filename = argv[2];
        
        if (0 == strcmp(argv[1],"-s"))
        {
            model = modelcreatorCreate("CUBE");
            loaderSaveModel(model,filename);
            
            return 0;
        }

        if (0 == strcmp(argv[1],"-l"))
        {
            err = plotterLoadModelFromFile(filename);
        }
    }
    else
    {
        err = plotterAddModel(modelcreatorCreate("CUBE"),NULL);
    }
    
    if (0 <= err)
    {
        gv_runlevel = 1;
        err = loopViewer(&gv_runlevel);
    }
    else
    {
        ERROR1("Could not find a model\n");
    }
        
    modelstoreClear();
    
    finalizeCommen();
    
    LOG("Leaving %s\n",GV_APPLICATION);
        
    return err;
}



