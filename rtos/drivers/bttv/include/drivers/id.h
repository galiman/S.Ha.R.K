
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* FIXME: this temporarely, until these are included in linux/i2c-id.h */

/* drivers */
#ifndef  I2C_DRIVERID_TVMIXER
# define I2C_DRIVERID_TVMIXER I2C_DRIVERID_EXP0
#endif
#ifndef  I2C_DRIVERID_TVAUDIO
# define I2C_DRIVERID_TVAUDIO I2C_DRIVERID_EXP1
#endif

/* chips */
#ifndef  I2C_DRIVERID_DPL3518
# define I2C_DRIVERID_DPL3518 I2C_DRIVERID_EXP2
#endif
#ifndef  I2C_DRIVERID_TDA9873
# define I2C_DRIVERID_TDA9873 I2C_DRIVERID_EXP3
#endif
#ifndef  I2C_DRIVERID_TDA9875
# define I2C_DRIVERID_TDA9875 I2C_DRIVERID_EXP0+4
#endif
#ifndef  I2C_DRIVERID_PIC16C54_PV951
# define I2C_DRIVERID_PIC16C54_PV951 I2C_DRIVERID_EXP0+5
#endif
#ifndef  I2C_DRIVERID_TDA7432
# define I2C_DRIVERID_TDA7432 I2C_DRIVERID_EXP0+6
#endif
#ifndef  I2C_DRIVERID_TDA9874
# define I2C_DRIVERID_TDA9874 I2C_DRIVERID_EXP0+7
#endif

/* algorithms */
#ifndef I2C_ALGO_SAA7134
# define I2C_ALGO_SAA7134 0x090000
#endif

