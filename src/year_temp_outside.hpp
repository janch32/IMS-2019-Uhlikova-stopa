#pragma once

/**
 * Naměřené teploty v roce 2018
 * 
 * Teploty jsou měřeny 3x denně - ráno 7h, odpoledne 14h, večer 21h.
 * Index v poli = konkrétní den - 1, př. pro dvacátý den je index 19
 * 
 * @see http://www.meteo.jankovic.cz/zaznamy/rok-2018/
 */
class OutsideTemp
{
public:
	/** Počet dnů v roce */
	static const int days = 365;

	/**
	 * Získat teplotu v konkrétním čase v roce. 
	 * Hodnoty teplot jsou lineárně interpolovány
	 * 
	 * @param day Pro který den chceme získat teplotu (indexace od 1)
	 * @param seconds V kterém čase (v sekundách)
	 * @return Interpolovaná teplota z naměřených dat
	 */
	static double getTemp(int day, int seconds);
private:
	/** Naměřená teplota v 7h ráno */
	static double morning[];
	
	/** Naměřená teplota v 14h odpoledne */
	static double noon[];
	
	/** Naměřená teplota v 21h večer */
	static double evening[];
};
