----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 12/27/2022 05:31:41 PM
-- Design Name: 
-- Module Name: led_ring - Behavioral
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

entity led_ring is
  Port (
    clk : in std_logic;
    mode : in std_logic_vector(1 downto 0);
    leds : out std_logic_vector(3 downto 0));
end led_ring;

architecture Behavioral of led_ring is
  signal d, dini : std_logic_vector(3 downto 0);
  signal load : std_logic;
  signal mode_reg : std_logic_vector(1 downto 0);
begin
  dini <= "0011" when mode = "01" else
          "0101" when mode = "10" else
          "0000";
  load <= '0' when mode = mode_reg else '1';
 
  process (clk)
  begin
    if clk = '1' and clk'event then
      mode_reg <= mode;
    end if;
  end process;

  process (clk)
  begin
    if clk = '1' and clk'event then
      if load = '1' then
        d <= dini;
      else
        d <= d(2 downto 0) & d(3);
      end if;
    end if;
  end process;
  leds <= d;
end Behavioral;


