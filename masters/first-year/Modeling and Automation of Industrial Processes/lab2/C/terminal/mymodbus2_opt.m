function ret= mymodbus2_opt( op, a1 )

global MMB2
if isempty(MMB2)
    if exist('mymodbus2_opt_local.m', 'file')
        % use it if exists mymodbus2_opt_local.m
        MMB2= mymodbus2_opt_local();
    else
        % otherwise fill in defaults
        MMB2= MMB2_defaults();
    end
end

switch op
    case 'ip'
        if nargin>1
            % for people that confuse 'ip' and 'set_ip'
            MMB2.ip= a1;   % a1= ip= string
        end
        ret= MMB2.ip;      % mymodbus2_opt('ip')
    %case 'ip',   ret= MMB2.ip;      % mymodbus2_opt('ip')

    case 'port', ret= MMB2.port;    % mymodbus2_opt('port')

    case 'set_ip',   MMB2.ip= a1;   % a1= ip= string
    case 'set_port', MMB2.port= a1; % a1= port= 502
end

% frequent usage:
% mymodbus2_opt('ip'), mymodbus2_opt('port')

return; % end of main function


function MMB2= MMB2_defaults()
MMB2= struct('ip', '127.0.0.1', 'port', 502);
