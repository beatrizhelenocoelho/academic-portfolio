function [ret, ret2]= myterminal4_aux( cmd, a1, a2, a3, a4 )
%
% This function exists to take code out of function myterminal4.m
%
% Usage example:
% myterminal4_aux( 'set_coils', 0, zeros(1:5) )
% myterminal4_aux( 'get_coils', 0, 10 )

% Mar2020, JG

if nargin<1
    myterminal4;
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
        if nargin>= 3
            % call from the fig callback
            myterminal_refresh( a1, a2 ); % ( hObject, handles )
        else
            % command line call, refresh during 5sec
            h= myterminal_options( 'get', 'handles', [] );
            if ~isfield(h, 'REFRESH')
                warning('lost terminal fig data, please reopen the fig');
                return
            end
            myterminal_options( 'set', 'timeout', datenum(now+seconds(5)) );
            myterminal_refresh( h.REFRESH, h );
            myterminal_options( 'set', 'timeout', inf);
        end
        
    case 'addr_cnf'
        % get offset addresses  myterminal4_aux('addr_cnf',0,[])
        ret= plc_comm_addresses_config( a1, a2 );
    case 'options'
        % ret= myterminal_options( op, a1, a2 )
        ret= myterminal_options( a1, a2, a3 );
    case 'log_strings'
        % ret= myterminal4_aux('log_strings', 'get', [])
        % ret= log_strings( op, str )
        ret= log_strings( a1, a2 );

    case 'mymenu'
        % GUI to select options by their names
        mymenu
    case 'mymenu2'
        % command line to select options by their names
        mymenu( a1 )
        
    otherwise
        error('inv cmd');
end


% ----------------------------------------------------------------------
function ret= plc_comm_addresses_config( op, a1 )
% get offset addresses  ADR= plc_comm_addresses_config(0);
% zero offset addresses myterminal4_aux('addr_cnf',-1,[])
% zero offset addresses myterminal4_aux('addr_cnf',1,[0 10 10 0 20])
% higher def addresses  myterminal4_aux('addr_cnf',1,[200 10 10 200 20])

% Address translation options defined in the first version of myterminal
ADR_default= [180 10 10 180 70]; %M180..189, %M190..199, %MW180..249

% Save options across multiple calls
persistent ADR
if isempty(ADR)
    %ADR= [180 10 10 180 20]; %M180..189, %M190..199, %MW180..199
    %ADR= [180 10 10 180 70]; %M180..189, %M190..199, %MW180..249
    ADR= ADR_default;
end

ret= [];
switch op
    case 0, ret= ADR; % return current adresses translation
    case 1, ADR= a1;  % set addresses translation, a1 is 1x5

    case -1 
        % ADR= [0 10 10 0 20]; %M0..9, %M10..19, %MW0..19
        ADR= [0 10 10 0 70]; %M0..9, %M10..19, %MW0..69
        % ADR= [0 10 10 180 70]; %M0..9, %M10..19, %MW180..169

    case 100, ADR= ADR_default;
    case 101, ADR= [180 10 10 180 70];
    case 102, ADR= [0 10 10 180 70];
    case 103, ADR= [0 10 10 0 70];

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
m= mymodbus2('ini');

coilValues= [];
if ~isempty(nCoils) && nCoils>0
    %coilValues= myread(m, 'coils', firstCoil, nCoils);
    if ~addr_ok( addr.outpCoilsFirst, addr.outpCoilsNum, firstCoil, nCoils)
       warning('outp coil(s) addr out of range')
    end
    coilValues= mymodbus2('read', m, 'coils', firstCoil +addr.outpCoilsFirst, nCoils);
end

regValues= [];
if ~isempty(nRegs) && nRegs>0
    %regValues= myread(m, 'holdingregs', firstReg, nRegs );
    if ~addr_ok( addr.regsFirst, addr.regsNum, firstReg, nRegs)
       warning('reg(s) addr out of range')
    end
    regValues= mymodbus2('read', m, 'holdingregs', firstReg +addr.regsFirst, nRegs);
end

mymodbus2('end', m);


function set_coils_and_regs( firstCoil, coilValues, firstReg, regValues )
addr= plc_comm_addresses;
m= mymodbus2('ini');

if ~isempty(coilValues)
    %mywrite( m, 'coils', firstCoil, coilValues );
    if ~addr_ok( addr.inpCoilsFirst, addr.inpCoilsNum, firstCoil, length(coilValues))
       warning('inp coil(s) addr out or range')
    end
    firstCoil2= firstCoil +addr.inpCoilsFirst;
    mymodbus2('write', m, 'coils', firstCoil2, coilValues );
end

if ~isempty(regValues)
    %mywrite( m, 'holdingregs', firstReg, regValues );
    if ~addr_ok( addr.regsFirst, addr.regsNum, firstReg, length(regValues))
       warning('reg(s) addr out or range')
    end
    mymodbus2('write', m, 'holdingregs', firstReg +addr.regFirst, regValues );
end

mymodbus2('end', m);


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
    % enforce not going over end of comms buffer
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
function ret= myterminal_options( op, a1, a2 )

persistent MTO
if isempty(MTO)
    MTO= struct('refreshDebug',0, 'refreshPeriod', 0.1, ...
        'keyPressedDuration',1, 'logStrings',0, 'logStringsDebug',0, ...
        'timeout', inf);
end

ret= [];
switch op
    case 'getAll', ret= MTO;
    case 'get'
        if isfield( MTO, a1 )
            ret= getfield( MTO, a1 );
        else
            warning('field not found %s', a1)
            ret= [];
        end
        
    case 'set'
        MTO= setfield(MTO, a1, a2);

    case 'setRefreshDebug'
        % myterminal_options( 'setRefreshDebug', 1 )
        % myterminal_options( 'setRefreshDebug', 0 )
        myterminal_options( 'set', 'refreshDebug', a1 );

    case 'setRefreshPeriod'
        % myterminal_options( 'setRefreshPeriod', 1 )
        % myterminal_options( 'setRefreshPeriod', 0.1 )
        myterminal_options( 'set', 'refreshPeriod', a1 );
        
    case 'setKeyPressedDuration'
        myterminal_options( 'set', 'keyPressedDuration', a1 );

    case 'setLogStrings'
        myterminal_options( 'set', 'logStrings', a1 );
        
    otherwise
        warning('inv op %s', op)
end


function ret= log_strings( op, str )
persistent LS
persistent LS_last_str
if isempty(LS)
    LS= {};
end

ret= LS;

switch op
    case 'get'
        % do nothing, just return LS
    case 'show'
        if isempty(LS)
            msgbox('** no strings logged till now **', 'Logged strings')
        else
            msgbox(LS, 'Logged strings')
        end
        
    case 'reset'
        LS= {};
        log_strings( 'show' );

    case 'push'
        % command line show events (debug)
        if ~strcmp(str, LS_last_str) && ...
                myterminal_options( 'get', 'logStringsDebug' )
            fprintf(1, 'new string: %s\n', str);
            LS_last_str= str;
        end
        
        % if current string equals the previously saved, just do nothing
        if length(LS)>0 && strcmp( str, LS{end} )
            return
        end
        
        % there is some novelty, str may need to be saved in the list
        switch myterminal_options( 'get', 'logStrings' )
            case 0
                % do nothing
            case 1
                % push every novel "tmp" str
                LS{end+1,1}= str;
            case 2
                % just keep final strings (overwrite the incomplete ones)
                if isempty(LS)
                    LS{1,1}= str;
                elseif isempty(LS{end,1}) || strncmp( str, LS{end,1}, length(LS{end,1}) )
                    LS{end,1}= str;
                else
                    LS{end+1,1}= str;
                end
        end
        
    otherwise
        error('inv op');
end


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

timeout= myterminal_options('get', 'timeout'); % inf
if ~isinf(timeout) || get(hObject, 'Value') %rem(cnt,2)~=0
    set( hObject, 'String', 'Running' );
end

refreshPeriod= myterminal_options('get', 'refreshPeriod');
debugFlag= myterminal_options('get', 'refreshDebug'); %0; %1;
loopNum= 0;
while (1)
    loopNum= loopNum+1;
    if debugFlag
        fprintf(1, 'refresh cnt=%d loopNum=%d\n', cnt, loopNum);
    end
    if (now >= timeout) || (isinf(timeout) && ~get(hObject, 'Value'))
        %rem(cnt,2)==0
        if ~isinf(timeout), datevec(now), datevec(timeout), end
        set( hObject, 'String', 'Paused' );
        return
    end
    
    % -- Get PLC output bits:
    
    %[read_outputs, read_mode] = myterminal4_aux( 'get_coils_and_regs', 301,5, 1,1 );
    %[read_outputs, read_mode] = myterminal4_aux( 'get_coils_and_regs', 0,4, 0,1 );
    read_outputs = myterminal4_aux( 'get_coils', 0,4 );
    
    set( handles.BUZZER,        'Value', read_outputs(1) );
    set( handles.RED_LED,       'Value', read_outputs(2) );
    set( handles.YELLOW_LED,    'Value', read_outputs(3) );
    set( handles.GREEN_LED,     'Value', read_outputs(4) );

    % -- Get PLC words (including one string):
    
    read_mode = myterminal4_aux( 'get_regs', 0,30 );
    if exist('myterminal4_txt.m', 'file')
        str= myterminal4_txt( read_mode(1) );
    else
        str= sprintf('Mode %d', read_mode(1) );
    end
    set( handles.edit9, 'String', str );
    
    strFromThePLC= message2string( read_mode );
    set( handles.text2display, 'String', strFromThePLC );
    log_strings( 'push', strFromThePLC );
    
    %pause(0.1)
    pause(refreshPeriod)
end


function strFromThePLC= message2string( read_mode )
% ignore first 10 values of read_mode
% find a string cropping at the first zero
x= read_mode(11:end);
x= [rem(x(:),256) round(x(:)/256)]';
x= x(:);
ind= find(x==0); ind= [ind; length(x)];
x= x(1:ind(1))';
strFromThePLC= char(x);


% ----------------------------------------------------------------------
function keyshandler( newKey )

persistent KH
persistent KHinfo

if isempty(KH)
    KH= {};
    %KH= {11, datenum(now+seconds(0.07))};
end

% save in a list (i) current key pressed and (ii) its death time, i.e. key-up simulation
%
KH{end+1,1}= newKey;
% KH{end,2}= datenum(now+seconds(0.1));
% KH{end,2}= datenum(now+seconds(1));
keyPressedDuration= myterminal_options('get', 'keyPressedDuration');
KH{end,2}= datenum(now+seconds(keyPressedDuration));
% fprintf(1, '#keys=%d\n', size(KH,1));

% loop get columns using modbus
%
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
        myterminal4_aux('set_coils', 4,[0 0 0 0]);
        KHinfo= [];
        break
    end
    
    % loop get columns using modbus
    cols= myterminal4_aux('get_coils',4,3);
    lins= calc_lines( cell2mat({KH{:,1}}), cols );

    % if columns or lines did change then save cols and send lines
    if isempty(KHinfo) || max(abs(cols-KHinfo.cols)) || ...
            ~isfield(KHinfo, 'lins') || max(abs(lins-KHinfo.lins))
        KHinfo.cols= cols;
        if ~isfield(KHinfo, 'lins') || max(abs(lins-KHinfo.lins))
            myterminal4_aux('set_coils', 4,lins);
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


% ----------------------------------------------------------------------
function mymenu( cmdId )
if nargin<1
    cmdId= '';
end

% List of available menu options
PCAC= 'plc_comm_addresses_config';
cmd= {
    'Modbus debug ON',   'mymodbus2( "db_level_set", 1 )'; ...
    'Modbus debug Off',  'mymodbus2( "db_level_set", 0 )'; ...
    'Modbus debug show', 'fprintf(1, "modbus dbLevel=%d\n", mymodbus2( "db_level", "get", [] ))'; ...
    '---', ''; ...
    'Comms addr base set def',       [PCAC '( 100 );']; ...
    'Comms addr base set m0 mw0',    [PCAC '( 103 );']; ...
    'Comms addr base set m0 mw180',  [PCAC '( 102 );']; ...
    'Comms addr base set m180 mw180',[PCAC '( 101 );']; ...
    'Comms addr base get cnf',       ['addrTbl= ' PCAC '( 0 )']; ...
    '---', ''; ...
    'Key-down duration set 0.1sec', 'myterminal_options( "setKeyPressedDuration", 0.1 );'; ...
    'Key-down duration set 1sec',   'myterminal_options( "setKeyPressedDuration", 1 );'; ...
    '---', ''; ...
    'Log strings reset',     'log_strings("reset");'; ...
    'Log tmp strings',       'myterminal_options( "setLogStrings", 1 );'; ...
    'Log final strings',     'myterminal_options( "setLogStrings", 2 );'; ...
    'Pause logging strings', 'myterminal_options( "setLogStrings", 0 );'; ...
    'Logged strings show',   'log_strings("show");'; ...
    '---', ''; ...
    'LS debug ON',  'myterminal_options( "set", "logStringsDebug", 1 );'; ...
    'LS debug Off', 'myterminal_options( "set", "logStringsDebug", 0 );'; ...
    'Refresh debug ON',  'myterminal_options( "setRefreshDebug", 1 );'; ...
    'Refresh debug Off', 'myterminal_options( "setRefreshDebug", 0 );'; ...
    'Refresh period 1sec',   'myterminal_options( "setRefreshPeriod", 1 );'; ...
    'Refresh period 0.1sec', 'myterminal_options( "setRefreshPeriod", 0.1 );'; ...
    'Refresh options show',  'refreshOptions= myterminal_options( "getAll" )'; ...
    };
for i=1:size(cmd,1)
    cmd{i,2}= strrep( cmd{i,2}, '"', '''' );
end

% Define what is to be done
if isempty(cmdId)
    % ask user what is to be done
    [indList, okFlag] = listdlg('PromptString',...
        {'Select action to do' ,'(press Cancel to avoid action):'},...
        'SelectionMode', 'multiple',... %'single', ... %'multiple',...
        'ListSize', [300 370], ...
        'ListString', cmd(:,1) );
else
    % cmdId should match an entry in cmd(:,1), e.g. cmd{5,1}
    okFlag= 0;
    for i=1:size(cmd,1)
        if strcmp(cmd{i,1}, cmdId)
            indList= i; okFlag= 1;
            break
        end
    end
end
if ~okFlag
    return
end
    
% Do the work
for i= indList
    if ~strcmp(cmd{i,1}, '---');
    eval( cmd{i,2} );
    end
end
