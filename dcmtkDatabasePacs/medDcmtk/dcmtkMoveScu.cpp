#include "dcmtkMoveScu.h"

#include "dcmtk/ofstd/ofstdinc.h"
#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmnet/dicom.h"
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/dcmdata/dcmetinf.h"
#include "dcmtk/dcmdata/dcdict.h"

#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcuid.h"      /* for dcmtk version name */
#include "dcmtk/dcmdata/dcdicent.h"
#include "dcmtk/dcmdata/dcostrmz.h"   /* for dcmZlibCompressionLevel */

#include "dcmtkLogger.h"

//---------------------------------------------------------------------------------------------

dcmtkMoveScu::dcmtkMoveScu()
{
    opt_maxPDU = ASC_DEFAULTMAXPDU;
    opt_useMetaheader = OFTrue;
    opt_acceptAllXfers = OFFalse;
    opt_networkTransferSyntax = EXS_Unknown;
    opt_writeTransferSyntax = EXS_Unknown;
    opt_in_networkTransferSyntax = EXS_Unknown;
    opt_out_networkTransferSyntax = EXS_Unknown;
    opt_groupLength = EGL_recalcGL;
    opt_sequenceType = EET_ExplicitLength;
    opt_paddingType = EPD_withoutPadding;
    opt_filepad = 0;
    opt_itempad = 0;
    opt_compressionLevel = 0;
    opt_bitPreserving = OFFalse;
    opt_ignore = OFFalse;
    opt_abortDuringStore = OFFalse;
    opt_abortAfterStore = OFFalse;
    opt_correctUIDPadding = OFFalse;
    opt_repeatCount = 1;
    opt_abortAssociation = OFFalse;
    opt_moveDestination = NULL;
    opt_cancelAfterNResponses = -1;
    opt_queryModel = QMPatientRoot;
    opt_ignorePendingDatasets = OFTrue;
    opt_outputDirectory = ".";
    

    overrideKeys = NULL;
    m_useBuildInStoreScp = false;

    this->setQueryLevel(STUDY);

}

//---------------------------------------------------------------------------------------------

int  dcmtkMoveScu::sendMoveRequest(const char* peerTitle, const char* peerIP, int peerPort, 
                                   const char* targetTitle, const char* targetIP, int targetPort)
{

    // always use study info model
    opt_queryModel = QMStudyRoot;

    if (overrideKeys == NULL) 
    {
        dcmtkLogger::errorStream() << "You have to add keys to look for!";
    }

    T_ASC_Parameters *params = NULL;
    const char *opt_peer = peerIP;
    OFCmdUnsignedInt opt_port = peerPort;
    opt_retrievePort = targetPort;
    DIC_NODENAME localHost;
    DIC_NODENAME peerHost;
    T_ASC_Association *assoc = NULL;
    const char *opt_peerTitle = peerTitle;
    const char *opt_ourTitle = targetTitle;
    OFList<OFString> fileNameList;

    QuerySyntax querySyntax[3] =  {
                { UID_FINDPatientRootQueryRetrieveInformationModel,
                  UID_MOVEPatientRootQueryRetrieveInformationModel },
                { UID_FINDStudyRootQueryRetrieveInformationModel,
                  UID_MOVEStudyRootQueryRetrieveInformationModel },
                { UID_FINDPatientStudyOnlyQueryRetrieveInformationModel,
                  UID_MOVEPatientStudyOnlyQueryRetrieveInformationModel }
                };


    /* make sure output directory exists and is writeable */
    if (opt_retrievePort > 0)
    {
        if (!OFStandard::dirExists(opt_outputDirectory))
        {
          dcmtkLogger::errorStream() << "specified output directory does not exist";
          return 1;
        }
        else if (!OFStandard::isWriteable(opt_outputDirectory))
        {
          dcmtkLogger::errorStream() << "specified output directory is not writeable";
          return 1;
        }
    }


    /* network for move request and responses */
    T_ASC_NetworkRole role = (opt_retrievePort > 0) ? NET_ACCEPTORREQUESTOR : NET_REQUESTOR;
    OFCondition cond = ASC_initializeNetwork(role, OFstatic_cast(int, opt_retrievePort), opt_acse_timeout, &net);
    if (cond.bad())
    {
        dcmtkLogger::errorStream() << "cannot create network: " << DimseCondition::dump(temp_str, cond);
        return 1;
    }

    /* set up main association */
    cond = ASC_createAssociationParameters(&params, opt_maxPDU);
    if (cond.bad()) {
        dcmtkLogger::errorStream() << DimseCondition::dump(temp_str, cond);
        return 1;
    }
    ASC_setAPTitles(params, opt_ourTitle, opt_peerTitle, NULL);

    gethostname(localHost, sizeof(localHost) - 1);
    sprintf(peerHost, "%s:%d", opt_peer, (int)opt_port);
    ASC_setPresentationAddresses(params, localHost, peerHost);

    /*
     * We also add a presentation context for the corresponding
     * find sop class.
     */
    cond = addPresentationContext(params, 1,
        querySyntax[opt_queryModel].findSyntax, opt_networkTransferSyntax);

    cond = addPresentationContext(params, 3,
        querySyntax[opt_queryModel].moveSyntax, opt_networkTransferSyntax);
    if (cond.bad()) {
        dcmtkLogger::errorStream() << DimseCondition::dump(temp_str, cond);
        return 1;
    }

    dcmtkLogger::debugStream() << "Request Parameters:"; 
         dcmtkLogger::debugStream() << ASC_dumpParameters(temp_str, params, ASC_ASSOC_RQ);

   // CreateAssociation(cond, net,params, assoc);

    /* create association */
    dcmtkLogger::infoStream() << "Requesting Association";
    cond = ASC_requestAssociation(net, params, &assoc);
    if (cond.bad()) {
        if (cond == DUL_ASSOCIATIONREJECTED) {
            T_ASC_RejectParameters rej;

            ASC_getRejectParameters(params, &rej);
            dcmtkLogger::errorStream() << "Association Rejected:";
                dcmtkLogger::errorStream() << ASC_printRejectParameters(temp_str, &rej);
            return 1;
        } else {
            dcmtkLogger::errorStream() << "Association Request Failed:";
                dcmtkLogger::errorStream() << DimseCondition::dump(temp_str, cond);
            return 1;
        }
    }
    /* what has been accepted/refused ? */
    dcmtkLogger::debugStream() << "Association Parameters Negotiated:"; 
        dcmtkLogger::debugStream() << ASC_dumpParameters(temp_str, params, ASC_ASSOC_AC);

    if (ASC_countAcceptedPresentationContexts(params) == 0) {
        dcmtkLogger::errorStream() << "No Acceptable Presentation Contexts";
        return 1;
    }

    dcmtkLogger::infoStream() << "Association Accepted (Max Send PDV: " << assoc->sendPDVLength << ")";

    /* do the real work */
    cond = EC_Normal;
    if (fileNameList.empty())
    {
      /* no files provided on command line */
      cond = cmove(assoc, NULL);
    } else {
      OFListIterator(OFString) iter = fileNameList.begin();
      OFListIterator(OFString) enditer = fileNameList.end();
      while ((iter != enditer) && cond.good())
      {
          cond = cmove(assoc, (*iter).c_str());
          ++iter;
      }
    }

    //ReleaseAssociation(cond);

            /* tear down association */
    if (cond == EC_Normal)
    {
        if (opt_abortAssociation) {
            dcmtkLogger::infoStream() << "Aborting Association";
            cond = ASC_abortAssociation(assoc);
            if (cond.bad()) {
                dcmtkLogger::errorStream() << "Association Abort Failed: " << DimseCondition::dump(temp_str, cond);
                return 1;
            }
        } else {
            /* release association */
            dcmtkLogger::infoStream() << "Releasing Association";
            cond = ASC_releaseAssociation(assoc);
            if (cond.bad())
            {
                dcmtkLogger::errorStream() << "Association Release Failed:";
                    dcmtkLogger::errorStream() << DimseCondition::dump(temp_str, cond);
                return 1;
            }
        }
    }
    else if (cond == DUL_PEERREQUESTEDRELEASE)
    {
        dcmtkLogger::errorStream() << "Protocol Error: Peer requested release (Aborting)";
        dcmtkLogger::infoStream() << "Aborting Association";
        cond = ASC_abortAssociation(assoc);
        if (cond.bad()) {
            dcmtkLogger::errorStream() << "Association Abort Failed: " << DimseCondition::dump(temp_str, cond);
            return 1;
        }
    }
    else if (cond == DUL_PEERABORTEDASSOCIATION)
    {
        dcmtkLogger::infoStream() << "Peer Aborted Association";
    }
    else
    {
        dcmtkLogger::errorStream() << "Move SCU Failed: " << DimseCondition::dump(temp_str, cond);
        dcmtkLogger::infoStream() << "Aborting Association";
        cond = ASC_abortAssociation(assoc);
        if (cond.bad()) {
            dcmtkLogger::errorStream() << "Association Abort Failed: " << DimseCondition::dump(temp_str, cond);
            return 1;
        }
    }

    cond = ASC_destroyAssociation(&assoc);
    if (cond.bad()) {
        dcmtkLogger::errorStream() << DimseCondition::dump(temp_str, cond);
        return 1;
    }
    cond = ASC_dropNetwork(&net);
    if (cond.bad()) {
        dcmtkLogger::errorStream() << DimseCondition::dump(temp_str, cond);
        return 1;
    }


    return 0;
}

//---------------------------------------------------------------------------------------------

int dcmtkMoveScu::sendMoveRequest()
{
    return this->sendMoveRequest(opt_peerTitle, opt_peer, opt_port, 
                                 opt_ourTitle, opt_ourIP, opt_retrievePort);
}

//---------------------------------------------------------------------------------------------

OFCondition dcmtkMoveScu::acceptSubAssoc(T_ASC_Network * aNet, T_ASC_Association ** assoc)
{
    const char* knownAbstractSyntaxes[] = {
        UID_VerificationSOPClass
    };
    const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    int numTransferSyntaxes;

    OFCondition cond = ASC_receiveAssociation(aNet, assoc, ASC_DEFAULTMAXPDU);
    if (cond.good())
    {
      switch (EXS_Unknown)
      {
        case EXS_LittleEndianImplicit:
          /* we only support Little Endian Implicit */
          transferSyntaxes[0] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 1;
          break;
        case EXS_LittleEndianExplicit:
          /* we prefer Little Endian Explicit */
          transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 3;
          break;
        case EXS_BigEndianExplicit:
          /* we prefer Big Endian Explicit */
          transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 3;
          break;
        case EXS_JPEGProcess14SV1TransferSyntax:
          /* we prefer JPEGLossless:Hierarchical-1stOrderPrediction (default lossless) */
          transferSyntaxes[0] = UID_JPEGProcess14SV1TransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 4;
          break;
        case EXS_JPEGProcess1TransferSyntax:
          /* we prefer JPEGBaseline (default lossy for 8 bit images) */
          transferSyntaxes[0] = UID_JPEGProcess1TransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 4;
          break;
        case EXS_JPEGProcess2_4TransferSyntax:
          /* we prefer JPEGExtended (default lossy for 12 bit images) */
          transferSyntaxes[0] = UID_JPEGProcess2_4TransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 4;
          break;
        case EXS_JPEG2000LosslessOnly:
          /* we prefer JPEG2000 Lossless */
          transferSyntaxes[0] = UID_JPEG2000LosslessOnlyTransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 4;
          break;
        case EXS_JPEG2000:
          /* we prefer JPEG2000 Lossy */
          transferSyntaxes[0] = UID_JPEG2000TransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 4;
          break;
        case EXS_JPEGLSLossless:
          /* we prefer JPEG-LS Lossless */
          transferSyntaxes[0] = UID_JPEGLSLosslessTransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 4;
          break;
        case EXS_JPEGLSLossy:
          /* we prefer JPEG-LS Lossy */
          transferSyntaxes[0] = UID_JPEGLSLossyTransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 4;
          break;
        case EXS_MPEG2MainProfileAtMainLevel:
          /* we prefer MPEG2 MP@ML */
          transferSyntaxes[0] = UID_MPEG2MainProfileAtMainLevelTransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 4;
          break;
        case EXS_RLELossless:
          /* we prefer RLE Lossless */
          transferSyntaxes[0] = UID_RLELosslessTransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 4;
          break;
#ifdef WITH_ZLIB
        case EXS_DeflatedLittleEndianExplicit:
          /* we prefer Deflated Explicit VR Little Endian */
          transferSyntaxes[0] = UID_DeflatedExplicitVRLittleEndianTransferSyntax;
          transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
          transferSyntaxes[2] = UID_BigEndianExplicitTransferSyntax;
          transferSyntaxes[3] = UID_LittleEndianImplicitTransferSyntax;
          numTransferSyntaxes = 4;
          break;
#endif
        default:
          if (OFFalse)
          {
            /* we accept all supported transfer syntaxes
             * (similar to "AnyTransferSyntax" in "storescp.cfg")
             */
            transferSyntaxes[0] = UID_JPEG2000TransferSyntax;
            transferSyntaxes[1] = UID_JPEG2000LosslessOnlyTransferSyntax;
            transferSyntaxes[2] = UID_JPEGProcess2_4TransferSyntax;
            transferSyntaxes[3] = UID_JPEGProcess1TransferSyntax;
            transferSyntaxes[4] = UID_JPEGProcess14SV1TransferSyntax;
            transferSyntaxes[5] = UID_JPEGLSLossyTransferSyntax;
            transferSyntaxes[6] = UID_JPEGLSLosslessTransferSyntax;
            transferSyntaxes[7] = UID_RLELosslessTransferSyntax;
            transferSyntaxes[8] = UID_MPEG2MainProfileAtMainLevelTransferSyntax;
            transferSyntaxes[9] = UID_DeflatedExplicitVRLittleEndianTransferSyntax;
            if (gLocalByteOrder == EBO_LittleEndian)
            {
              transferSyntaxes[10] = UID_LittleEndianExplicitTransferSyntax;
              transferSyntaxes[11] = UID_BigEndianExplicitTransferSyntax;
            } else {
              transferSyntaxes[10] = UID_BigEndianExplicitTransferSyntax;
              transferSyntaxes[11] = UID_LittleEndianExplicitTransferSyntax;
            }
            transferSyntaxes[12] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 13;
          } else {
            /* We prefer explicit transfer syntaxes.
             * If we are running on a Little Endian machine we prefer
             * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
             */
            if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
            {
              transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
              transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
            } else {
              transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
              transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
            }
            transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
            numTransferSyntaxes = 3;
          }
          break;

        }

        /* accept the Verification SOP Class if presented */
        cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
            (*assoc)->params,
            knownAbstractSyntaxes, DIM_OF(knownAbstractSyntaxes),
            transferSyntaxes, numTransferSyntaxes);

        if (cond.good())
        {
            /* the array of Storage SOP Class UIDs comes from dcuid.h */
            cond = ASC_acceptContextsWithPreferredTransferSyntaxes(
                (*assoc)->params,
                dcmAllStorageSOPClassUIDs, numberOfAllDcmStorageSOPClassUIDs,
                transferSyntaxes, numTransferSyntaxes);
        }
    }
    if (cond.good()) cond = ASC_acknowledgeAssociation(*assoc);
    if (cond.bad()) {
        ASC_dropAssociation(*assoc);
        ASC_destroyAssociation(assoc);
    }
    return cond;
}

//---------------------------------------------------------------------------------------------

OFCondition dcmtkMoveScu::echoSCP(T_ASC_Association * assoc,
                                T_DIMSE_Message * msg,
                                T_ASC_PresentationContextID presID)
{
  OFString temp_str;
  dcmtkLogger::infoStream() << "Received Echo Request";
  dcmtkLogger::debugStream() << DIMSE_dumpMessage(temp_str, msg->msg.CEchoRQ, DIMSE_INCOMING);

  /* the echo succeeded !! */
  OFCondition cond = DIMSE_sendEchoResponse(assoc, presID, &msg->msg.CEchoRQ, STATUS_Success, NULL);
  if (cond.bad())
  {
    dcmtkLogger::errorStream() << "Echo SCP Failed: " << DimseCondition::dump(temp_str, cond);
  }
  return cond;
}

//---------------------------------------------------------------------------------------------

void dcmtkMoveScu::substituteOverrideKeys(DcmDataset *dset)
{
    if (overrideKeys == NULL) {
        return; /* nothing to do */
    }

    /* copy the override keys */
    DcmDataset keys(*overrideKeys);

    /* put the override keys into dset replacing existing tags */
    unsigned long elemCount = keys.card();
    for (unsigned long i = 0; i < elemCount; i++) {
        DcmElement *elem = keys.remove((unsigned long)0);

        dset->insert(elem, OFTrue);
    }
}

//---------------------------------------------------------------------------------------------

bool dcmtkMoveScu::processQueryAttribute(const char* key)
{
    unsigned int g = 0xffff;
    unsigned int e = 0xffff;
    int n = 0;
    char val[1024];
    OFString dicName, valStr;
    OFString msg;
    char msg2[200];
    val[0] = '\0';

    // try to parse group and element number
    n = sscanf(key, "%x,%x=%s", &g, &e, val);
    OFString toParse = key;
    size_t eqPos = toParse.find('=');
    if (n < 2)  // if at least no tag could be parsed
    {
      // if value is given, extract it (and extrect dictname)
      if (eqPos != OFString_npos)
      {
        dicName = toParse.substr(0,eqPos).c_str();
        valStr = toParse.substr(eqPos+1,toParse.length());
      }
      else // no value given, just dictionary name
        dicName = key; // only dictionary name given (without value)
      // try to lookup in dictionary
      DcmTagKey key(0xffff,0xffff);
      const DcmDataDictionary& globalDataDict = dcmDataDict.rdlock();
      const DcmDictEntry *dicent = globalDataDict.findEntry(dicName.c_str());
      dcmDataDict.unlock();
      if (dicent!=NULL) {
        // found dictionary name, copy group and element number
        key = dicent->getKey();
        g = key.getGroup();
        e = key.getElement();
      }
      else {
        // not found in dictionary
        msg = "bad key format or dictionary name not found in dictionary: ";
        msg += dicName;
        dcmtkLogger::errorStream() << msg;
        return false;
      }
    } // tag could be parsed, copy value if it exists
    else
    {
      if (eqPos != OFString_npos)
        valStr = toParse.substr(eqPos+1,toParse.length());
    }
    DcmTag tag(g,e);
    if (tag.error() != EC_Normal) {
        sprintf(msg2, "unknown tag: (%04x,%04x)", g, e);
        dcmtkLogger::errorStream() << msg2;
        return false;
    }
    DcmElement *elem = newDicomElement(tag);
    if (elem == NULL) {
        sprintf(msg2, "cannot create element for tag: (%04x,%04x)", g, e);
        dcmtkLogger::errorStream() << msg2;
        return false;
    }
    if (valStr.length() > 0) {
        if (elem->putString(valStr.c_str()).bad())
        {
            sprintf(msg2, "cannot put tag value: (%04x,%04x)=\"", g, e);
            msg = msg2;
            msg += valStr;
            msg += "\"";
            dcmtkLogger::errorStream() << msg2;
            return false;
        }
    }

    if (overrideKeys == NULL) 
            overrideKeys = new DcmDataset;
    if (overrideKeys->insert(elem, OFTrue).bad()) {
        sprintf(msg2, "cannot insert tag: (%04x,%04x)", g, e);
        dcmtkLogger::errorStream() << msg2;
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------

OFCondition dcmtkMoveScu::moveSCU(T_ASC_Association * assoc, const char *fname)
{
    T_ASC_PresentationContextID presId;
    T_DIMSE_C_MoveRQ    req;
    T_DIMSE_C_MoveRSP   rsp;
    DIC_US              msgId = assoc->nextMsgID++;
    DcmDataset          *rspIds = NULL;
    const char          *sopClass;
    DcmDataset          *statusDetail = NULL;
    MyCallbackInfo      callbackData;

    QuerySyntax querySyntax[3] =  {
            { UID_FINDPatientRootQueryRetrieveInformationModel,
              UID_MOVEPatientRootQueryRetrieveInformationModel },
            { UID_FINDStudyRootQueryRetrieveInformationModel,
              UID_MOVEStudyRootQueryRetrieveInformationModel },
            { UID_FINDPatientStudyOnlyQueryRetrieveInformationModel,
              UID_MOVEPatientStudyOnlyQueryRetrieveInformationModel }
            };


    DcmFileFormat dcmff;

    if (fname != NULL) {
        if (dcmff.loadFile(fname).bad()) {
            dcmtkLogger::errorStream() << "bad DICOM file: " << fname << ": " << dcmff.error().text();
            return DIMSE_BADDATA;
        }
    }

    /* replace specific keys by those in overrideKeys */
    substituteOverrideKeys(dcmff.getDataset());

    sopClass = querySyntax[opt_queryModel].moveSyntax;

    /* which presentation context should be used */
    presId = ASC_findAcceptedPresentationContextID(assoc, sopClass);
    if (presId == 0) return DIMSE_NOVALIDPRESENTATIONCONTEXTID;

    dcmtkLogger::infoStream() << "Sending Move Request: MsgID " << msgId;
        dcmtkLogger::infoStream() << "Request:"; 
            dcmtkLogger::infoStream() << DcmObject::PrintHelper(*dcmff.getDataset());


    callbackData.assoc = assoc;
    callbackData.presId = presId;

    req.MessageID = msgId;
    strcpy(req.AffectedSOPClassUID, sopClass);
    req.Priority = DIMSE_PRIORITY_MEDIUM;
    req.DataSetType = DIMSE_DATASET_PRESENT;
    if (opt_moveDestination == NULL) {
        /* set the destination to be me */
        ASC_getAPTitles(assoc->params, req.MoveDestination,
            NULL, NULL);
    } else {
        strcpy(req.MoveDestination, opt_moveDestination);
    }

    OFCondition cond;

    if(m_useBuildInStoreScp)
    {
        cond = DIMSE_moveUser(assoc, presId, &req, dcmff.getDataset(),
                                          moveCallback, (void*) this, opt_blockMode,
                                          opt_dimse_timeout, net, subOpCallback,
                                          (void*) this, &rsp, &statusDetail, &rspIds, opt_ignorePendingDatasets);
    }
    else
    {
        cond = DIMSE_moveUser(assoc, presId, &req, dcmff.getDataset(),
                                          moveCallback, (void*) this, opt_blockMode,
                                          opt_dimse_timeout, net, NULL,
                                          (void*) this, &rsp, &statusDetail, &rspIds, opt_ignorePendingDatasets);
    }

    if (cond == EC_Normal) {
        OFString temp_str;
        dcmtkLogger::infoStream() << DIMSE_dumpMessage(temp_str, rsp, DIMSE_INCOMING);
        if (rspIds != NULL) {
            dcmtkLogger::infoStream() << "Response Identifiers:";
                dcmtkLogger::infoStream() << DcmObject::PrintHelper(*rspIds);
        }
    } else {
        OFString temp_str;
        dcmtkLogger::errorStream() << "Move Request Failed: " << DimseCondition::dump(temp_str, cond);
    }
    if (statusDetail != NULL) {
        dcmtkLogger::warningStream() << "Status Detail:";
            dcmtkLogger::warningStream() << DcmObject::PrintHelper(*statusDetail);
        delete statusDetail;
    }

    if (rspIds != NULL) delete rspIds;

    return cond;
}

//---------------------------------------------------------------------------------------------

OFCondition dcmtkMoveScu::cmove(T_ASC_Association * assoc, const char *fname)
{
    OFCondition cond = EC_Normal;
    int n = (int)opt_repeatCount;
    while (cond.good() && n--)
        cond = moveSCU(assoc, fname);
    return cond;
}

//---------------------------------------------------------------------------------------------

OFCondition dcmtkMoveScu::storeSCP( T_ASC_Association *assoc,
                                  T_DIMSE_Message *msg,
                                  T_ASC_PresentationContextID presID,
                                  void* subOpCallbackData)
{
   

    // get context
    dcmtkMoveScu* scu = (dcmtkMoveScu*) subOpCallbackData;

    
    
    OFCondition cond = EC_Normal;
    T_DIMSE_C_StoreRQ *req;
    char imageFileName[2048];

    req = &msg->msg.CStoreRQ;

    if (OFFalse)
    {
#ifdef _WIN32
        tmpnam(imageFileName);
#else
        strcpy(imageFileName, NULL_DEVICE_NAME);
#endif
    } else {
        sprintf(imageFileName, "%s.%s",
            dcmSOPClassUIDToModality(req->AffectedSOPClassUID),
            req->AffectedSOPInstanceUID);
    }

    OFString temp_str;
    dcmtkLogger::infoStream() << "Received Store Request: MsgID " << req->MessageID << ", ("
        << dcmSOPClassUIDToModality(req->AffectedSOPClassUID, "OT") << ")";
    dcmtkLogger::debugStream() << DIMSE_dumpMessage(temp_str, *req, DIMSE_INCOMING, NULL, presID);

    scu->m_assoc = assoc;
    scu->m_imageFileName = imageFileName;
    DcmFileFormat dcmff;
    scu->m_dcmff = &dcmff;

    // store SourceApplicationEntityTitle in metaheader
    if (assoc && assoc->params)
    {
      const char *aet = assoc->params->DULparams.callingAPTitle;
      if (aet) dcmff.getMetaInfo()->putAndInsertString(DCM_SourceApplicationEntityTitle, aet);
    }

    DcmDataset *dset = dcmff.getDataset();

    if (OFFalse)
    {
      cond = DIMSE_storeProvider(assoc, presID, req, imageFileName, OFFalse,
          NULL, storeSCPCallback, (void *) subOpCallbackData, DIMSE_BLOCKING, 0);
    } else {
      cond = DIMSE_storeProvider(assoc, presID, req, (char *)NULL, OFFalse,
        &dset, storeSCPCallback, (void *) subOpCallbackData, DIMSE_BLOCKING, 0);
    }

    if (cond.bad())
    {
      dcmtkLogger::errorStream() << "Store SCP Failed: " << DimseCondition::dump(temp_str, cond);
      /* remove file */
      if (!OFFalse)
      {
        if (strcmp(imageFileName, NULL_DEVICE_NAME) != 0) unlink(imageFileName);
      }
#ifdef _WIN32
    } else if (OFFalse) {
        if (strcmp(imageFileName, NULL_DEVICE_NAME) != 0) unlink(imageFileName); // delete the temporary file
#endif
    }

    if (0 > 0)
    {
      OFStandard::sleep((unsigned int)0);
    }
    return cond;
}

//---------------------------------------------------------------------------------------------

OFCondition dcmtkMoveScu::subOpSCP(T_ASC_Association **subAssoc, void * subOpCallbackData)
{
    T_DIMSE_Message     msg;
    T_ASC_PresentationContextID presID;

    if (!ASC_dataWaiting(*subAssoc, 0)) /* just in case */
        return DIMSE_NODATAAVAILABLE;

    OFCondition cond = DIMSE_receiveCommand(*subAssoc, DIMSE_BLOCKING, 0, &presID,
            &msg, NULL);

    if (cond == EC_Normal) {
      switch (msg.CommandField)
      {
        case DIMSE_C_STORE_RQ:
          cond = storeSCP(*subAssoc, &msg, presID, subOpCallbackData);
          break;
        case DIMSE_C_ECHO_RQ:
          cond = echoSCP(*subAssoc, &msg, presID);
          break;
        default:
          cond = DIMSE_BADCOMMANDTYPE;
          dcmtkLogger::errorStream() << "cannot handle command: 0x"
               << STD_NAMESPACE hex << OFstatic_cast(unsigned, msg.CommandField);
          break;
      }
    }
    /* clean up on association termination */
    if (cond == DUL_PEERREQUESTEDRELEASE)
    {
        cond = ASC_acknowledgeRelease(*subAssoc);
        ASC_dropSCPAssociation(*subAssoc);
        ASC_destroyAssociation(subAssoc);
        return cond;
    }
    else if (cond == DUL_PEERABORTEDASSOCIATION)
    {
    }
    else if (cond != EC_Normal)
    {
        OFString temp_str;
        dcmtkLogger::errorStream() << "DIMSE failure (aborting sub-association): " << DimseCondition::dump(temp_str, cond);
        /* some kind of error so abort the association */
        cond = ASC_abortAssociation(*subAssoc);
    }

    if (cond != EC_Normal)
    {
        ASC_dropAssociation(*subAssoc);
        ASC_destroyAssociation(subAssoc);
    }
    return cond;
}

//---------------------------------------------------------------------------------------------

void dcmtkMoveScu::storeSCPCallback(  /* in */
                                    void *callbackData,
                                    T_DIMSE_StoreProgress *progress,    /* progress state */
                                    T_DIMSE_C_StoreRQ *req,             /* original store request */
                                    char *imageFileName, DcmDataset **imageDataSet, /* being received into */
                                    /* out */
                                    T_DIMSE_C_StoreRSP *rsp,            /* final store response */
                                    DcmDataset **statusDetail)
{
    if (!callbackData)
    return;

    // get context
    dcmtkMoveScu* scu = (dcmtkMoveScu*) callbackData;

    DIC_UI sopClass;
    DIC_UI sopInstance;

    if ((scu->opt_abortDuringStore && progress->state != DIMSE_StoreBegin) ||
        (scu->opt_abortAfterStore && progress->state == DIMSE_StoreEnd)) {
        dcmtkLogger::infoStream() << "ABORT initiated (due to command line options)";
        ASC_abortAssociation(scu->m_assoc);
        rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;
        return;
    }

    if (scu->opt_sleepDuring > 0)
    {
      OFStandard::sleep((unsigned int)scu->opt_sleepDuring);
    }
    
    // TODO find substitute here - mk
    /*
    // dump some information if required (depending on the progress state)
    // We can't use oflog for the pdu output, but we use a special logger for
    // generating this output. If it is set to level "INFO" we generate the
    // output, if it's set to "DEBUG" then we'll assume that there is debug output
    // generated for each PDU elsewhere.
    OFLogger progressLogger = OFLog::getLogger("progress");
    if (progressLogger.getChainedLogLevel() == OFLogger::INFO_LOG_LEVEL)
    {
      switch (progress->state)
      {
        case DIMSE_StoreBegin:
          COUT << "RECV: ";
          break;
        case DIMSE_StoreEnd:
          COUT << OFendl;
          break;
        default:
          COUT << '.';
          break;
      }
      COUT.flush();
    }
    */

    if (progress->state == DIMSE_StoreEnd)
    {
       *statusDetail = NULL;    /* no status detail */

       /* could save the image somewhere else, put it in database, etc */
       /*
        * An appropriate status code is already set in the resp structure, it need not be success.
        * For example, if the caller has already detected an out of resources problem then the
        * status will reflect this.  The callback function is still called to allow cleanup.
        */
       

       if ((imageDataSet != NULL) && (*imageDataSet != NULL) && !scu->opt_bitPreserving && !scu->opt_ignore)
       {
         /* create full path name for the output file */
         OFString ofname;
         OFStandard::combineDirAndFilename(ofname, scu->opt_outputDirectory, scu->m_imageFileName, OFTrue /* allowEmptyDirName */);

         E_TransferSyntax xfer = scu->opt_writeTransferSyntax;
         if (xfer == EXS_Unknown) xfer = (*imageDataSet)->getOriginalXfer();

         
         OFCondition cond = scu->m_dcmff->saveFile(ofname.c_str(), xfer, scu->opt_sequenceType, scu->opt_groupLength,
           scu->opt_paddingType, OFstatic_cast(Uint32, scu->opt_filepad), OFstatic_cast(Uint32, scu->opt_itempad),
           (scu->opt_useMetaheader) ? EWM_fileformat : EWM_dataset);
         if (cond.bad())
         {
           dcmtkLogger::errorStream() << "cannot write DICOM file: " << ofname;
           rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;
         }

        /* should really check the image to make sure it is consistent,
         * that its sopClass and sopInstance correspond with those in
         * the request.
         */
        if ((rsp->DimseStatus == STATUS_Success) && !scu->opt_ignore)
        {
          /* which SOP class and SOP instance ? */
          if (!DU_findSOPClassAndInstanceInDataSet(*imageDataSet, sopClass, sopInstance, scu->opt_correctUIDPadding))
          {
             dcmtkLogger::errorStream() << "bad DICOM file: " << imageFileName;
             rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
          }
          else if (strcmp(sopClass, req->AffectedSOPClassUID) != 0)
          {
            rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
          }
          else if (strcmp(sopInstance, req->AffectedSOPInstanceUID) != 0)
          {
            rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
          }
        }
      }
      else
      {
        dcmtkLogger::errorStream() << "cannot write file - unknown reason";
      }
    }
 
}

//---------------------------------------------------------------------------------------------

void dcmtkMoveScu::subOpCallback(void * subOpCallbackData ,
                               T_ASC_Network *aNet, T_ASC_Association **subAssoc)
{
    if (!subOpCallbackData)
        return;



    if (aNet == NULL) return;   /* help no net ! */

    if (*subAssoc == NULL) {
        /* negotiate association */
        acceptSubAssoc(aNet, subAssoc);
    } else {
        /* be a service class provider */
        subOpSCP(subAssoc, subOpCallbackData);
    }
}

//---------------------------------------------------------------------------------------------

void dcmtkMoveScu::moveCallback(void *callbackData, T_DIMSE_C_MoveRQ *request,
                              int responseCount, T_DIMSE_C_MoveRSP *response)
{
    if (!callbackData)
    return;

    // get context
    dcmtkMoveScu* scu = (dcmtkMoveScu*) callbackData;

    OFCondition cond = EC_Normal;
    MyCallbackInfo *myCallbackData;

    myCallbackData = (MyCallbackInfo*)callbackData;

    OFString temp_str;
    dcmtkLogger::infoStream() << "Move Response " << responseCount << ":"; 
        dcmtkLogger::infoStream() << DIMSE_dumpMessage(temp_str, *response, DIMSE_INCOMING);

    /* should we send a cancel back ?? */
    if (scu->opt_cancelAfterNResponses == responseCount) {
        dcmtkLogger::infoStream() << "Sending Cancel Request: MsgID " << request->MessageID
                 << ", PresID " << myCallbackData->presId;
        cond = DIMSE_sendCancelRequest(myCallbackData->assoc,
            myCallbackData->presId, request->MessageID);
        if (cond != EC_Normal) {
            dcmtkLogger::errorStream() << "Cancel Request Failed: " << DimseCondition::dump(temp_str, cond);
        }
    }
}

//---------------------------------------------------------------------------------------------

void dcmtkMoveScu::useBuildInStoreSCP(bool flag)
{
    m_useBuildInStoreScp = flag;
}

//---------------------------------------------------------------------------------------------

dcmtkMoveScu::~dcmtkMoveScu()
{
    if (overrideKeys != NULL)
       delete overrideKeys;
}

//---------------------------------------------------------------------------------------------

void dcmtkMoveScu::clearAllQueryAttributes()
{
  overrideKeys->clear();
}

//---------------------------------------------------------------------------------------------
