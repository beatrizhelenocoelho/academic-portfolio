function tu_sim_kb( tstId )
%
% This function sets some defaults for keyboard testing using PNs

% 13.5.2021, J. Gaspar

if nargin<1
    tstId= 1;
end

global keys_pressed;

switch tstId
    case -1
        % show current seq of keys:  tu_sim_kb(-1)
        PN_device_kb_IO
        
    case 0
        % reset default seq of keys exp:  tu_sim_kb(0)
        keys_pressed= [];
        PN_device_kb_IO

    case 1
        % set test all keys:  tu_sim_kb(1)
        %global keys_pressed; keys_pressed= tu_sim('nbits_one_by_one', 12);
        keys_pressed= tu_sim('nbits_one_by_one', 12);

    otherwise
        error('inv tstId')
end
