----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 09/13/2016 07:01:44 PM
-- Design Name: 
-- Module Name: fpga_basicIO - Behavioral
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
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity fpga_board_IO is
  port (
    clk: in std_logic;                            -- 100MHz clock
    btnC, btnU, btnL, btnR, btnD: in std_logic;   -- buttons
    sw: in std_logic_vector(15 downto 0);         -- switches
    led: out std_logic_vector(15 downto 0);       -- leds
    an: out std_logic_vector(3 downto 0);         -- display selectors
    seg: out std_logic_vector(6 downto 0);        -- display 7-segments
    dp: out std_logic                             -- display point
  );
end fpga_board_IO;

architecture Behavioral of fpga_board_IO is
  signal dd3, dd2, dd1, dd0 : std_logic_vector(6 downto 0);
  signal res, reg1 : std_logic_vector(7 downto 0);
  signal dact : std_logic_vector(3 downto 0);
  signal en_10hz, en_disp : std_logic;
  signal btn, btnDeBnc : std_logic_vector(4 downto 0);
  signal btnCreg, btnUreg, btnLreg, btnRreg, btnDreg: std_logic;   -- registered input buttons
  signal sw_reg : std_logic_vector(15 downto 0);  -- registered input switches
  signal A_modulo, B_modulo : std_logic_vector(4 downto 0);
  signal saida_comp2, saida_modulo : std_logic_vector(7 downto 0);
  
  component disp7
  port (
    digit3, digit2, digit1, digit0 : in std_logic_vector(3 downto 0);
    dp3, dp2, dp1, dp0 : in std_logic;
    clk : in std_logic;
    dactive : in std_logic_vector(3 downto 0);
    en_disp_l : out std_logic_vector(3 downto 0);
    segm_l : out std_logic_vector(6 downto 0);
    dp_l : out std_logic);
  end component;
  
  component debouncer
  generic (
    DEBNC_CLOCKS : integer;
    PORT_WIDTH : integer);
  port (
    signal_i : in std_logic_vector(4 downto 0);
    clk_i : in std_logic;          
    signal_o : out std_logic_vector(4 downto 0));
  end component;
  
  component Lab3 
    Port ( A : in STD_LOGIC_VECTOR (4 downto 0);
           B : in STD_LOGIC_VECTOR (4 downto 0);
           I : in STD_LOGIC;
           S : out STD_LOGIC_VECTOR (7 downto 0));
  end component;

begin
  -- Modificar apenas a instanciacao abaixo do componente lab0 (para a do componente com 3 entradas)
  inst_circuito: lab3 port map ( A => sw_reg(15 downto 11), B => sw_reg(4 downto 0),
                                 I => sw_reg(7), S => saida_comp2 );
                
               
  led(15 downto 11) <= sw_reg(15 downto 11);
  led(10 downto 8) <= (others => '0');
  led(7) <= sw_reg(7);
  led(6 downto 5) <= (others => '0');
  led(4 downto 0) <= sw_reg(4 downto 0);
  
  dact <= "1111";
    
  with saida_comp2(7) select
    saida_modulo <= ((not saida_comp2) + 1) when '1',
                    saida_comp2 when others; 
  with sw_reg(15) select
    A_modulo <= ((not sw_reg(15 downto 11)) + 1) when '1',
                sw_reg(15 downto 11) when others;                    
  with sw_reg(4) select
    B_modulo <= ((not sw_reg(4 downto 0)) + 1) when '1',
                sw_reg(4 downto 0) when others;  
       
  inst_disp7: disp7 port map(
      digit3 => saida_modulo(7 downto 4), digit2 => saida_modulo(3 downto 0), 
      digit1 => A_modulo(3 downto 0), digit0 => B_modulo(3 downto 0),
      dp3 => saida_comp2(7), dp2 => '0', dp1 => sw_reg(15), dp0 => sw_reg(4),  
      clk => clk,
      dactive => dact, 
      en_disp_l => an,
      segm_l => seg,
      dp_l => dp);
 
--Debounces btn signals
  btn <= btnC & btnU & btnL & btnR & btnD;
  Inst_btn_debounce: debouncer 
    generic map (
        DEBNC_CLOCKS => (2**16),
        PORT_WIDTH => 5)
    port map (
		signal_i => btn,
		clk_i => clk,
		signal_o => btnDeBnc );
         
  process (clk)
    begin
       if rising_edge(clk) then
           btnCreg <= btnDeBnc(4); 
           btnUreg <= btnDeBnc(3); 
           btnLreg <= btnDeBnc(2); 
           btnRreg <= btnDeBnc(1); 
           btnDreg <= btnDeBnc(0);
           sw_reg <= sw;
       end if; 
    end process;    
end Behavioral;
