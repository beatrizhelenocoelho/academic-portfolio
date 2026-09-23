function mem_dump_load_tst(tstId)
if nargin<1
    tstId= 1;
end

switch tstId
    case 1
        fname= 'Dados\datalog_a3_sim.DTX';
        x= mem_dump_load( fname );

        figure(201); clf;
        stairs(x(1,:), x(2,:), '.-', 'linewidth',4)
        xlabel('scan cycle number')
        ylabel('16bits word as decimal')
        
        figure(202); clf;
        z= dec2bin(x(2,:),16)-'0'; z= z(:,end:-1:1);
        %plot_z(x(1,:), z)
        %plot_z(x(1,:), z, struct('patch',1))
        plot_z(x(1,:), z, struct('patch',1,'zoh',1))
        xlabel('scan cycle number')
        ylabel('bit number')
        
    otherwise
        error('inv tstId')
end

return
