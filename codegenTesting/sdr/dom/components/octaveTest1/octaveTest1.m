# this is a comment that should be ignored
## this is another comment that should be ignored
% This is yet another comment that should be ignored
function [vectorOut0, stringPropIO1, stringPropOutput1, complexPropIO1, complexPropOutput1, complexVectorOutput1, complexVectorPropIO1, vectorPropIO1, vectorPropOutput1] = octaveTest0(vectorIn0,stringPropOutput1="default",constIn0=0,stringPropIO1="", __sampleRate,complexPropIO1=0, emptyStringSeqProp=[""], stringSeqProp=["", "1", "12"], vectorPropIO1=[0j,1],vectorPropOutput1=[],complexVectorPropIO1=[],complexVectorOutput1=[],complexPropOutput1=0i )

    tmp = octaveDependency("foo");
    if tmp ~= "foo"
        return
    end

    % test0_vectorData
    vectorOut0           = vectorIn0 + constIn0;

    % test1_configureQuery
    stringPropOutput1   = "output";
    stringPropIO1       = stringPropIO1;
    complexPropIO1      = complexPropIO1;
    complexPropOutput1  = 1+2j;

    vectorPropIO1        = vectorPropIO1;
    vectorPropOutput1    = [-1, 0, 1, 2.5];
    complexVectorPropIO1 = complexVectorPropIO1;
    complexVectorOutput1 = [-1-1j, 0, 1, 2+2i];

    stringSeqProp = stringSeqProp;
    emptyStringSeqProp = emptyStringSeqProp;
