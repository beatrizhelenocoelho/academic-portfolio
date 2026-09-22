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
  signal dact : std_logic_vector(3 downto 0);
  signal clk_1Hz : std_logic;
  signal btn, btnDeBnc : std_logic_vector(4 downto 0);
  signal btnCreg, btnUreg, btnLreg, btnRreg, btnDreg: std_logic;   -- registered input buttons
  signal sw_reg : std_logic_vector(15 downto 0);  -- registered input switches
  signal saida_Q : std_logic_vector(7 downto 0);
  signal saida_S, saida_T : std_logic_vector(3 downto 0);
    
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
  
  component led_ring 
    Port (
      clk : in std_logic;
      mode : in std_logic_vector(1 downto 0);
      leds : out std_logic_vector(3 downto 0));
  end component;    
  
  component phone_mgr 
    Port ( clk       : in STD_LOGIC;
           SUP       : in STD_LOGIC;
           ESQ       : in STD_LOGIC;
           DTO       : in STD_LOGIC;
           -- EstadoInit: in STD_LOGIC_VECTOR (7 downto 0);
           state_ME  : out STD_LOGIC_VECTOR (7 downto 0);
           saidas_ME : out STD_LOGIC_VECTOR (1 downto 0);
           timer_cnt : out STD_LOGIC_VECTOR (3 downto 0));
  end component;

begin
  inst_circuito: 
    phone_mgr port map (
     clk => clk_1Hz, SUP => btnUreg,
     ESQ => btnLreg, DTO => btnRreg, 
     state_ME => saida_Q, -- EstadoInit => "00000001",
     saidas_ME => saida_S(1 downto 0),
     timer_cnt => saida_T);
  saida_S(3 downto 2) <= "00";
    
  led(11 downto 8) <= (others => '0');
  led(7 downto 0) <= saida_Q(7 downto 0);
  inst_ledsr : led_ring port map (clk => clk_1Hz, mode => saida_S(1 downto 0), leds => led(15 downto 12));

  dact <= "1011";
  inst_disp7: disp7 port map(
      digit3 => saida_T, digit2 => saida_T, 
      digit1 => saida_Q(7 downto 4), digit0 => saida_Q(3 downto 0),
      dp3 => '0', dp2 => '0', dp1 => '0', dp0 => '0',  
      clk => clk,
      dactive => dact, 
      en_disp_l => an,
      segm_l => seg,
      dp_l => dp);
 
-- Debounces btn signals
  btn <= btnC & btnU & btnL & btnR & btnD;
    
  Inst_btn_debounce: debouncer 
    generic map (
        DEBNC_CLOCKS => (2**20),
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
