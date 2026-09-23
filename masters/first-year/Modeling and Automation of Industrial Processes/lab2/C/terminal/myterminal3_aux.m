function [ret, ret2]= myterminal3_aux( cmd, a1, a2, a3, a4 )
%
% This function exists to take code out of function myterminal3.m
%
% Usage example:
% myterminal3_aux( 'set_coils', 0, zeros(1:5) )
% myterminal3_aux( 'get_coils', 0, 10 )

% Mar2020, JG

if nargin<1
    myterminal3;
    return
end

ret= [];
switch cmd
    case 'get_coils'
        ret= get_coils( a1, a2 ); % ( firstCoil, numCoils );
    case 'get_regs'
        ret= get_regs( a1, a2 );
    case 'get_coils_and_regs'
        [ret, ret2]= get_coils_and_regs( a1, a2, a3, a4 );

    case 'set_regs'
        set_regs( a1, a2 );
    case 'set_coils'
        set_coils( a1, a2); % ( coilNum, value(s) );
    case 'set_coils_and_regs'
        set_coils_and_regs( a1, a2, a3, a4 );
        
    case 'key_push_plc_gui'
        % KeyId push to PLC and GUI
        key_push_plc_gui( a1, a2, a3); % ( keyId, resetFlag, handle )
    case 'key_push'
        keyshandler( a1 );
        
    case 'refresh'
        myterminal_refresh( a1, a2 ); % ( hObject, handles )
        
    case 'addr_cnf'
        % get offset addresses  myterminal3_aux('addr_cnf',0,[])
        ret= plc_comm_addresses_config( a1, a2 );
    otherwise
        error('inv cmd');
end


% ----------------------------------------------------------------------
function ret= plc_comm_addresses_config( op, a1 )
% get offset addresses  ADR= plc_comm_addresses_config(0);
% zero offset addresses myterminal3_aux('addr_cnf',-1,[])
% zero offset addresses myterminal3_aux('addr_cnf',1,[0 10 10 0 20])
% higher def addresses  myterminal3_aux('addr_cnf',1,[200 10 10 200 20])

persistent ADR
if isempty(ADR)
    ADR= [180 10 10 180 20]; %M180..189, %M190..199, %MW180..199
end
ret= [];
switch op
    case 0, ret= ADR;
    case 1, ADR= a1;
    case -1, ADR= [0 10 10 0 20]; %M0..9, %M10..19, %MW0..19
    otherwise, error('inv op');
end


function addr= plc_comm_addresses

% Define here the PLC memory usage for communications.
% This example uses high addresses in order to decrease the chance of
% conflicts with the programs runing in the PLC.

% The idea is to use near zero addresses everywhere but in the base access
% functions. So the user believes is using low addresses but in practice is
% using high addresses as the low addresses are offset at base comm functions.

% User lower addresses to work for TSX P57 1634M 02.00:
% addr= struct( ...
%     'inpCoilsFirst',  180, ... % first input coil at %M180
%     'inpCoilsNum',     10, ...
%     'outpCoilsFirst', 190, ... % first output coil at %M190
%     'outpCoilsNum',    10, ...
%     'regsFirst',      180, ... % first IO reg at %MW180
%     'regsNum',         20 );

ADR= plc_comm_addresses_config(0);
addr= struct( ...
    'inpCoilsFirst',  ADR(1), ... % first input coil
    'inpCoilsNum',    ADR(2), ...
    'outpCoilsFirst', sum(ADR(1:2)), ... % first output coil
    'outpCoilsNum',   ADR(3), ...
    'regsFirst',      ADR(4), ... % first IO reg
    'regsNum',        ADR(5));

% scan cycle first and last programs:
% for i=0:9, fprintf(1,'%%m%d:=%%m%d;\n', i, i+180); end
% for i=0:9, fprintf(1,'%%m%d:=%%m%d;\n', i+190, i+10); end


function flag= addr_ok( first, maxNum, first2, maxNum2 )
flag= 0;
a1= first; a2= first+maxNum-1;
b1= a1+first2; b2= a1+first2+maxNum2-1;
% test a1 <= b1, b2 <= a2
if a1<=b1 && b1<=a2 && a1<=b2 && b2<=a2
    flag= 1;
end


function [coilValues, regValues]= get_coils_and_regs( firstCoil, nCoils, firstReg, nRegs )
addr= plc_comm_addresses;
m= mymodbus('ini');

coilValues= [];
if ~isempty(nCoils) && nCoils>0
    %coilValues= myread(m, 'coils', firstCoil, nCoils);
    if ~addr_ok( addr.outpCoilsFirst, addr.outpCoilsNum, firstCoil, nCoils)
       warning('outp coil(s) addr out or range')
    end
    coilValues= mymodbus('read', m, 'coils', firstCoil +addr.outpCoilsFirst, nCoils);
end

regValues= [];
if ~isempty(nRegs) && nRegs>0
    %regValues= myread(m, 'holdingregs', firstReg, nRegs );
    if ~addr_ok( addr.regsFirst, addr.regsNum, firstReg, nRegs)
       warning('reg(s) addr out or range')
    end
    regValues= mymodbus('read', m, 'holdingregs', firstReg +addr.regsFirst, nRegs);
end

mymodbus('end', m);


function set_coils_and_regs( firstCoil, coilValues, firstReg, regValues )
addr= plc_comm_addresses;
m= mymodbus('ini');

if ~isempty(coilValues)
    %mywrite( m, 'coils', firstCoil, coilValues );
    if ~addr_ok( addr.inpCoilsFirst, addr.inpCoilsNum, firstCoil, length(coilValues))
       warning('inp coil(s) addr out or range')
    end
    firstCoil2= firstCoil +addr.inpCoilsFirst;
    mymodbus('write', m, 'coils', firstCoil2, coilValues );
end

if ~isempty(regValues)
    %mywrite( m, 'holdingregs', firstReg, regValues );
    if ~addr_ok( addr.regsFirst, addr.regsNum, firstReg, length(regValues))
       warning('reg(s) addr out or range')
    end
    mymodbus('write', m, 'holdingregs', firstReg +addr.regFirst, regValues );
end

mymodbus('end', m);


% ----------------------------------------------------------------------
function ret= get_coils( firstCoil, numCoils )
% m = modbus('tcpip', '127.0.0.1', 502);
% %read_outputs = myread(m,'coils',301,5);
% read_outputs = myread(m, 'coils', firstCoil, numCoils);
ret= get_coils_and_regs( firstCoil, numCoils, [], [] );


function set_coils( coilNum, values )
% m = modbus('tcpip', '127.0.0.1', 502);
% mywrite(m,'coils', coilNum, value); %mywrite at address %coilNum in Unity Pro
set_coils_and_regs( coilNum, values, [], [] );


function ret= get_regs( firstReg, nRegs )
[~, ret]= get_coils_and_regs( [], [], firstReg, nRegs );


function set_regs( firstReg, regValues )
set_coils_and_regs( [], [], firstReg, regValues );


% ----------------------------------------------------------------------
function push_key( keyId, resetFlag )
% save data into registers starting at 400
% entry index is registered at 499
if nargin<2
    resetFlag= 0;
end

% write where to write now
buffInd1= 1;
buffInd2= buffInd1+1;
ind= get_regs(buffInd1, 1); %Read at address %MW[1 +offset]
set_regs( buffInd2+ind, keyId );

% define where to write next
ind= ind+1;
if resetFlag
    ind= 0;
end
addr= plc_comm_addresses;
if ind > addr.regsNum-buffInd2-1
    % -buffInd2-1 = -3
    % buffer[0] is another variable
    % buffer[1] is the ind
    % the real buffer starts at buffer[2]
    ind= addr.regsNum-buffInd2-1;
end
set_regs( buffInd1, ind );


% ----------------------------------------------------------------------
function gui_strcat( handle, str )
% accumulate string
s1= [get(handle, 'String') str];
if length(s1)>10
    s1= s1(end-9:end);
end
% display the string
set(handle, 'String', s1);


function key_push_plc_gui( keyId, resetFlag, handle )
% put the key into the PLC and report it in the terminal
push_key( keyId, resetFlag );

% display the key on the screen
if keyId==10
    str= '*';
elseif keyId==11
    str= '#';
else
    str= num2str(keyId);
end
gui_strcat( handle, str );


% ----------------------------------------------------------------------
function myterminal_refresh( hObject, handles )
global hObjectSav cnt
hObjectSav= hObject;
if isempty(cnt)
    cnt=1;
else
    cnt= cnt+1;
end

% hObject =
%               Style: 'pushbutton'
%              String: 'Refresh'
%     BackgroundColor: [1 1 1]
%            Callback: [function_handle]
%               Value: 1
%            Position: [5.4444 2.6087 24.4444 3]
%               Units: 'characters'

if get(hObject, 'Value') %rem(cnt,2)~=0
    set( hObject, 'String', 'Running' );
end

debugFlag= 0; %1;
loopNum= 0;
while (1)
    loopNum= loopNum+1;
    if debugFlag
        fprintf(1, 'refresh cnt=%d loopNum=%d\n', cnt, loopNum);
    end
    if ~get(hObject, 'Value') %rem(cnt,2)==0
        set( hObject, 'String', 'Paused' );
        return
    end
    
    %[read_outputs, read_mode] = myterminal3_aux( 'get_coils_and_regs', 301,5, 1,1 );
    %[read_outputs, read_mode] = myterminal3_aux( 'get_coils_and_regs', 0,4, 0,1 );
    read_outputs = myterminal3_aux( 'get_coils', 0,4 );
    
    set( handles.BUZZER,        'Value', read_outputs(1) );
    set( handles.RED_LED,       'Value', read_outputs(2) );
    set( handles.YELLOW_LED,    'Value', read_outputs(3) );
    set( handles.GREEN_LED,     'Value', read_outputs(4) );
    
    read_mode = myterminal3_aux( 'get_regs', 0,1 );
    if exist('myterminal3_txt.m', 'file')
        str= myterminal3_txt( read_mode );
    else
        str= sprintf('Mode %d', read_mode);
    end
    set( handles.edit9, 'String', str );
    
    pause(0.5)
end


% ----------------------------------------------------------------------
function keyshandler( newKey )

persistent KH
persistent KHinfo

if isempty(KH)
    KH= {};
    %KH= {11, datenum(now+seconds(0.07))};
end
KH{end+1,1}= newKey;
% KH{end,2}= datenum(now+seconds(0.1));
KH{end,2}= datenum(now+seconds(1));
%fprintf(1, '#keys=%d\n', size(KH,1));

% loop get columns using modbus

while 1
    % remove timed-out keys from the buffer
    ind= [];
    tnow= now;
    for i= 1:size(KH,1)
        if KH{i,2} < tnow
            ind(end+1)= i;
        end
    end
    KH(ind,:)= [];
    if isempty(KH)
        % stop loop after no keys in buffer
        myterminal3_aux('set_coils', 4,[0 0 0 0]);
        KHinfo= [];
        break
    end
    
    % loop get columns using modbus
    cols= myterminal3_aux('get_coils',4,3);
    lins= calc_lines( cell2mat({KH{:,1}}), cols );

    % if columns or lines did change then save cols and send lines
    if isempty(KHinfo) || max(abs(cols-KHinfo.cols)) || ...
            ~isfield(KHinfo, 'lins') || max(abs(lins-KHinfo.lins))
        KHinfo.cols= cols;
        if ~isfield(KHinfo, 'lins') || max(abs(lins-KHinfo.lins))
            myterminal3_aux('set_coils', 4,lins);
            KHinfo.lins= lins;
        end
    end

end

return


function lins= calc_lines( keyList, cols )
% keyList: array of values in 0:11

% mark all active keys in a matrix 4x3
R= [1 2 3; 4 5 6; 7 8 9; 10 0 11];
M= zeros(4,3);
for k=1:length(keyList)
    [i,j]= find( keyList(k) == R );
    M(i,j)= 1;
end

% unmark unselected (unpowered) columns
% cols= [1 1 1]; % means all columns powered
for k= 1:length(cols)
    if ~cols(k)
        M(:,k)= 0;
    end
end

% return lines info
% lins= [0 0 0 0]; % if no keys pressed
lins= max(M,[],2)';
return
