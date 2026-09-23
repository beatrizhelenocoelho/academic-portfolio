function ret= login_opt( op, a1, a2 )
global LOPT
switch op
    case {'exist', 'isopt', 'isvar'}
        ret= isfield(LOPT, a1);
    case 'get'
        ret= LOPT.(a1);
    case 'set'
        LOPT.(a1)= a2;
    case 'show'
        % login_opt('show')
        disp(LOPT)
    otherwise
        error('inv op %s', op)
end
