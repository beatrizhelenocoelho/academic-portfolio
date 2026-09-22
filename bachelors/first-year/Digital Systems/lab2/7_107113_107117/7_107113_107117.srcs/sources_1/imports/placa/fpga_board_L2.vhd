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
  signal categoria, cat_sel : std_logic_vector(3 downto 0);
  signal digit_categ : std_logic_vector(7 downto 0);
  signal saida : std_logic;
  
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
  
  component lab2 
    Port ( A : in STD_LOGIC_VECTOR (3 downto 0);
           P : in STD_LOGIC_VECTOR (1 downto 0);
           C : out STD_LOGIC_VECTOR (3 downto 0);
           S : out STD_LOGIC);
  end component;

begin
  -- Modificar apenas a instanciacao abaixo do componente lab0 (para a do componente com 3 entradas)
  inst_circuito: lab2 port map ( A => sw_reg(3 downto 0), P => sw_reg(15 downto 14),
                                 C => categoria, S => saida );
                
               
  led(3 downto 0) <= sw_reg(3 downto 0);
  led(15 downto 14) <= sw_reg(15 downto 14);
  led(13 downto 10) <= (others => '0');
  led(9 downto 6) <= saida & saida & saida & saida ;
  led(5 downto 4) <= (others => '0');
   
  dact <= "1011";
    
  with categoria select
    digit_categ <= "11000000" when "0001",
                   "11000001" when "0010",
                   "11000010" when "0100",
                   "11000011" when "1000",
                   "00001010" when others; 
  cat_sel <= "00" & sw_reg(15 downto 14);
  inst_disp7: disp7 port map(
      digit3 => cat_sel, digit2 => sw_reg(3 downto 0), 
      digit1 => digit_categ(7 downto 4), digit0 => digit_categ(3 downto 0),
      dp3 => btnLreg, dp2 => btnDreg, dp1 => btnRreg, dp0 => btnUreg,  
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
