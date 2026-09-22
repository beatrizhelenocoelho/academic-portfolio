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

entity phone_mgr is
    Port ( clk       : in STD_LOGIC;
           SUP       : in STD_LOGIC;
           ESQ       : in STD_LOGIC;
           DTO       : in STD_LOGIC;
           -- EstadoInit: in STD_LOGIC_VECTOR (7 downto 0);
           state_ME  : out STD_LOGIC_VECTOR (7 downto 0);
           saidas_ME : out STD_LOGIC_VECTOR (1 downto 0);
           timer_cnt : out STD_LOGIC_VECTOR (3 downto 0));
end phone_mgr;

architecture Behavioral of phone_mgr is
-- componentes  
    component maq_estado 
    Port ( clk       : in STD_LOGIC;
           SUP       : in STD_LOGIC;
           ESQ       : in STD_LOGIC;
           DTO       : in STD_LOGIC;
           TOUT      : in STD_LOGIC;
           -- EstadoInit: in STD_LOGIC_VECTOR (7 downto 0);
           load_Tval : out STD_LOGIC_VECTOR (3 downto 0);
           enable_Tcnt : out STD_LOGIC;
           saida_ME  : out STD_LOGIC_VECTOR (1 downto 0);
           state_ME  : out STD_LOGIC_VECTOR (7 downto 0));
end component;

  component temporizador
    Port ( clk       : in STD_LOGIC;
           timer_en  : in STD_LOGIC;
           timer_val : in STD_LOGIC_VECTOR (3 downto 0);
           timer_cnt : out STD_LOGIC_VECTOR (3 downto 0);
           TOUT      : out STD_LOGIC);
  end component;
   
   signal estado_one_hot : STD_LOGIC_VECTOR (7 downto 0);
   signal saidas : STD_LOGIC_VECTOR (1 downto 0);
   signal tval : STD_LOGIC_VECTOR (3 downto 0);
   signal tenable, timer_out : STD_LOGIC;
    
begin

    --  NÃO MODIFICAR!!! 
	
    ME : maq_estado port map (
        clk => clk, SUP => SUP, ESQ => ESQ, DTO => DTO, 
        TOUT => timer_out, 
        -- estadoInit => estadoInit, 
        load_Tval => tval, enable_Tcnt => tenable,
        saida_ME => saidas, state_ME => estado_one_hot
    );
    
	
    --  NÃO MODIFICAR!!! 
    
    tempor : temporizador port map (
        clk => clk, timer_en => tenable, timer_cnt => timer_cnt,
        timer_val => tval,  TOUT => timer_out );
    
    --- NÃO MODIFICAR 
      
    state_ME <= estado_one_hot;
    saidas_ME <= saidas;
end Behavioral;
