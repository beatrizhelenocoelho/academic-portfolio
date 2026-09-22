----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 10.11.2016 10:47:20
-- Design Name: 
-- Module Name: float_add - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity maq_estado is
    Port ( clk       : in STD_LOGIC;
           SUP       : in STD_LOGIC;
           ESQ       : in STD_LOGIC;
           DTO       : in STD_LOGIC;
           TOUT      : in STD_LOGIC;
           -- EstadoInit: in STD_LOGIC_VECTOR (7 downto 0);
           load_Tval : out STD_LOGIC_VECTOR (3 downto 0);
           enable_Tcnt   : out STD_LOGIC;
           saida_ME  : out STD_LOGIC_VECTOR (1 downto 0);
           state_ME  : out STD_LOGIC_VECTOR (7 downto 0));
end maq_estado;

architecture Behavioral of maq_estado is

--componentes  -- NÃO MODIFICAR!

  component ff_de 
    Port ( din : in STD_LOGIC;
           clk : in STD_LOGIC;
           ce  : in STD_LOGIC;
           reset : in STD_LOGIC;
           set : in STD_LOGIC;
           dout : out STD_LOGIC);
  end component;
   
    -- sinais  -- NÃO MODIFICAR!
    signal E, D, T, nE, nD, nT : STD_LOGIC; 
    signal Dme, Qme        : STD_LOGIC_VECTOR (7 downto 0);
    signal Sff, nSff  : STD_LOGIC_VECTOR (7 downto 0) := (others => '0');
    
    -- Estado Inicial  -- MODIFICAR, quando pedido !
    constant EstadoInit : std_logic_vector(7 downto 0) := "00000001";
 
begin
     
    -- NÃO MODIFICAR!!! instanciacao de 8 FF_DE para a máquina de estados - NÃO MODIFICAR
inst_ffd_me: for i in 0 to 7 generate
    Sff(i) <= EstadoInit(i) and SUP;
    nSff(i) <= not(EstadoInit(i)) and SUP;
    FFD: ff_de port map (
        din => Dme(i), dout => Qme(i), 
        clk => clk,  reset => nSff(i), set => Sff(i), ce => '1'
    );
end generate;
    
    -- A MODIFICAR!!! Definição de estados one-hot REDEFINIR EM FUNÇÃO DE Qme(i), DTO, ESQ E TOUT
    -- Se o numero de FFs necessario for inferior a 8, deixe os restantes com as entradas a zero.    
      Dme(0) <= (Qme(1)and DTO) or  (Qme(4)and(DTO)) or (Qme(0)and not(DTO)) or (Qme(0) and DTO and ESQ) ;
     Dme(1) <= (Qme(1)and not(DTO) and not(TOUT)) or (Qme(2)and (DTO) and not(ESQ));
     Dme(2) <= (Qme(5)and TOUT) or  (Qme(6)and TOUT) or (Qme(2) and not(DTO) and not(ESQ) )or (Qme(0)and DTO and not(ESQ) ) or (Qme(2)and (DTO) and (ESQ)) or (Qme(6) and DTO and not(TOUT)) or (Qme(5) and DTO and not(TOUT)); 
     Dme(3) <= (Qme(3)and not(TOUT)) or (Qme(2)and not(DTO) and ESQ) ;
     Dme(4) <= (Qme(3)and TOUT) or (Qme(4) and not (DTO) and not (TOUT)); 
     Dme(5) <= (Qme(5)and not(TOUT) and not (DTO) ) or (Qme(1)and not(DTO)and TOUT) or (Qme(5)and not(DTO) and not(TOUT));
     Dme(6) <= (Qme(4) and not(DTO) and TOUT) or (Qme(6) and not(TOUT) and not(DTO)); 
     Dme(7) <= '0'; 

    -- A MODIFICAR!!! Saidas da ME REDEFINIR EM FUNï¿½ï¿½O DE Qme(i)
    
     load_Tval(3) <= Qme(1) or  Qme (3) or Qme(4) or Qme (5) or Qme(6);
     load_Tval(2) <= '0';
     load_Tval(1) <= '0';
     load_Tval(0) <= Qme(1) or  Qme (3) or Qme(4) or Qme (5) or Qme(6) ;
     enable_Tcnt  <= Qme(1) or Qme (3) or Qme(4) or Qme (5) or Qme(6);
     saida_ME(1)  <= Qme(3);
     saida_ME(0)  <= Qme(1);


    -- A MODIFICAR!!! Saidas da ME REDEFINIR EM FUNÇÃO DE Qme(i)
    
    

                                   
    --- Estado da Maquina - NÃO MODIFICAR 
	
    State_ME <= Qme;
 
end Behavioral;
