/*#
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
#*/

/*{% macro getPackets(port, mInputIndex) %}*/
// get data off the input port
inputPackets["${port.cppname}"] = ${port.cppname}->getPacket(-1);
if (not inputPackets["${port.cppname}"]) { 
    // No data is available because component is being killed
    return NOOP;
}

if (bufferingEnabled) {
    if (NOOP == buffer("${port.cppname}", ${port.cppname}) ) {
        return NOOP;
    }
}

/*{%- endmacro %}*/




/*{% macro defineInputProperty(prop, mInputIndex) %}*/
/*{% if prop['class'] == 'simple'%}*/
// add simple property to function input arguments
functionArguments.append(octave_value(${prop.cppname}));
//%  endif
/*{% if prop['class'] == 'simplesequence'%}*/
// std::vector -> octave matrix
/*{% if prop['cpptype'].find("std::complex") != -1 %}*/
ComplexRowVector ${prop.cppname}_octave(${prop.cppname}.size());
/*{% elif prop['cpptype'].find("string") != -1 %}*/
string_vector ${prop.cppname}_octave(${prop.cppname}.size());
//%  else
RowVector ${prop.cppname}_octave(${prop.cppname}.size());
//%  endif
for (unsigned int i=0; i<${prop.cppname}.size(); i++) {
    ${prop.cppname}_octave(i) = ${prop.cppname}[i];
}
// add simple sequence property to function input arguments
functionArguments.append(octave_value(${prop.cppname}_octave));
//%  endif

/*{%- endmacro %}*/





/*{% macro addInputArguments(component, functionInput, mInputIndex) %}*/
/*{% for port in component.ports%}*/
/*{%     if port.name == functionInput%}*/
appendInputPacketToFunctionArguments(
    functionArguments,
    inputPackets["${port.cppname}"]);
//%      endif
/*{% endfor %}*/
/*{% for prop in component.properties%}*/
/*{%     if prop.cppname == functionInput%}*/
${defineInputProperty(prop, mInputIndex)}
//%      endif
/*{% endfor %}*/
/*{% if functionInput == '__sampleRate'%}*/
functionArguments.append(octave_value(__sampleRate));
//%  endif
/*{%- endmacro %}*/



/*{% macro defineOutputProperty(prop, mOutputIndex) %}*/
/*{% if prop['class'] == 'simplesequence'%}*/
// store off sequence property values
// octave matrix -> octave vector
/*{% if prop['cpptype'].find("std::complex") != -1 %}*/
ComplexRowVector ${prop.cppname}_octaveRet(result(${mOutputIndex}).complex_array_value().length());
${prop.cppname}_octaveRet = result(${mOutputIndex}).complex_array_value();
/*{% elif prop['cpptype'].find("string") != -1 %}*/
string_vector ${prop.cppname}_octaveRet(result(${mOutputIndex}).cellstr_value().length());
${prop.cppname}_octaveRet = result(${mOutputIndex}).cellstr_value();
// Octave will add trailing whitespace to strings in the vector such
// that each element has the same length.  We will strip off this
// trailing whitespace.
for (int i = 0; i < ${prop.cppname}_octaveRet.length(); i++) {
    int end = ${prop.cppname}_octaveRet.elem(i).find_last_not_of(" ")+1;
    ${prop.cppname}_octaveRet.elem(i) = ${prop.cppname}_octaveRet.elem(i).substr(0, end);
}
//%  else
RowVector ${prop.cppname}_octaveRet(result(${mOutputIndex}).array_value().length());
${prop.cppname}_octaveRet = result(${mOutputIndex}).array_value();
//%  endif
// octave vector -> std::vector
${prop.cppname}.resize(${prop.cppname}_octaveRet.length());
for (unsigned int i = 0; i < ${prop.cppname}.size(); i++) {
    ${prop.cppname}[i] = ${prop.cppname}_octaveRet.elem(i);
}
//%  endif
/*{% if prop['class'] == 'simple'%}*/
// store off simple property value
/*{% if prop['iscomplex']%}*/
${prop.cppname} = result(${mOutputIndex}).complex_value();
/*{% elif prop['cpptype'] == 'std::string'%}*/
${prop.cppname} = result(${mOutputIndex}).string_value();
//%  else
${prop.cppname} = result(${mOutputIndex}).scalar_value();
//%  endif
//%  endif

/*{%- endmacro %}*/



/*{% macro defineOutputPort(port, mOutputIndex) %}*/
// convert output port data
populateOutputPacket(outputPackets["${port.cppname}"],
                     result,
                     ${mOutputIndex});
/*{%- endmacro %}*/



/*{% macro findOutputs(component, functionOutput, mOutputIndex) %}*/
/*{% for port in component.ports%}*/
/*{%     if port.name == functionOutput%}*/
${defineOutputPort(port, mOutputIndex)}
//%         endif
/*{% endfor %}*/
/*{% for prop in component.properties%}*/
/*{%     if prop.cppname == functionOutput%}*/
${defineOutputProperty(prop, mOutputIndex)}
/*{%     endif %}*/
/*{% endfor %}*/
/*{%- endmacro %}*/




/*{% macro octaveEmbedding(component) %}*/

// declarations
octave_value_list functionArguments; // pass to octave
octave_value_list result;            // pass from octave
int retVal;

/*{% for functionInput in component.mFunction.inputs%}*/
/*{%    for port in component.ports%}*/
/*{%        if port.name == functionInput%}*/
${getPackets(port, mInputIndex)}
//%         endif
/*{%    endfor %}*/
/*{% endfor %}*/

/*{%if component.ports%}*/
if (inputPackets.count(_sriPort) > 0) {
    __sampleRate = 1./(inputPackets[_sriPort]->SRI.xdelta);
}

/*{%endif%}*/
// Call to user-defined code.
retVal = preProcess();
if (retVal != NORMAL) {
    return retVal;
}

/*{% if component.mFunction.inputs%}*/

/*{% for functionInput in component.mFunction.inputs%}*/
${addInputArguments(component, functionInput, loop.index0)}
/*{% endfor %}*/
/*{% for vararginName in component.vararginList %}*/
${addInputArguments(component, vararginName, component.mFunction.numInputs+loop.index0-1)}
/*{% endfor %}*/

/*{% endif %}*/

setDiary();

// make the call to Octave
try {
    result = _feval(__mFunction.c_str(), functionArguments); 
} catch ( std::invalid_argument ) {
    return NORMAL;
}

flushDiary();

/*{% if component.mFunction.outputs%}*/

/*{% for functionOutput in component.mFunction.outputs%}*/
${findOutputs(component, functionOutput, loop.index0)}
/*{% endfor %}*/

/*{% endif %}*/

// Call to user-defined code.
retVal = postProcess();
if (retVal != NORMAL) {
    return retVal;
}

/*{% for port in component.ports%}*/
/*{%    for functionOutput in component.mFunction.outputs%}*/
/*{%        if port.name == functionOutput%}*/

if (outputPackets["${port.cppname}"]->sriChanged) {
    ${port.cppname}->pushSRI(outputPackets["${port.cppname}"]->SRI);
    outputPackets["${port.cppname}"]->sriChanged = false;
}

${port.cppname}->pushPacket(
        outputPackets["${port.cppname}"]->dataBuffer, /* data      */
        outputPackets["${port.cppname}"]->T,          /* timestamp */
        outputPackets["${port.cppname}"]->EOS,        /* EOS       */
        outputPackets["${port.cppname}"]->streamID);  /* stream ID */

//%         endif
/*{%    endfor %}*/
/*{% endfor %}*/

/*{% for port in component.ports%}*/
/*{%    for functionInput in component.mFunction.inputs%}*/
/*{%        if port.name == functionInput%}*/
delete inputPackets["${port.cppname}"];
//%         endif
/*{%    endfor %}*/
/*{% endfor %}*/

return _serviceFunctionReturnVal;

/*{%- endmacro %}*/

