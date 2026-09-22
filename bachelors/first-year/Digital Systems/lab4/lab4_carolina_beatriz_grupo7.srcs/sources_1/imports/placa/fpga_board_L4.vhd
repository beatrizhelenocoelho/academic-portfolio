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
  signal toggle_out, clk_1Hz : std_logic;
  signal btn, btnDeBnc : std_logic_vector(4 downto 0);
  signal btnCreg, btnUreg, btnLreg, btnRreg, btnDreg: std_logic;   -- registered input buttons
  signal sw_reg : std_logic_vector(15 downto 0);  -- registered input switches
  signal saida_Y, saida_S, saida_Q : std_logic_vector(3 downto 0);
    
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
  
  component clkdiv
    port(
      clk100M : in std_logic;          
      clk1Hz : out std_logic);
    end component;
      
  component lab4 
  Port ( 
   ini : in STD_LOGIC;
   clk : in STD_LOGIC;
   Y : out STD_LOGIC_VECTOR (3 downto 0);
   S : out STD_LOGIC_VECTOR (3 downto 0);
   Q : out STD_LOGIC_VECTOR (2 downto 0);
   t : out STD_LOGIC);
  end component;

begin
  inst_circuito: 
    lab4 port map ( ini => btnCreg, clk => clk_1Hz, Y => saida_Y, 
                    S => saida_S, Q => saida_Q(2 downto 0), t => toggle_out);
  saida_Q(3) <= '0';
    
  led(15 downto 0) <= (others => '0');
  
  dact <= "1011";
          
  inst_disp7: disp7 port map(
      digit3 => saida_Y, digit2 => sw_reg(3 downto 0), 
      digit1 => saida_S, digit0 => saida_Q,
      dp3 => '0', dp2 => '0', dp1 => toggle_out, dp0 => '0',  
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
    
    inst_clkdiv: clkdiv port map(
        clk100M => clk,
        clk1Hz => clk_1Hz);  
end Behavioral;
