#ifndef __BL_PROTOCOL_H
#define __BL_PROTOCOL_H

#define BL_PROTO_VER 0x0
#define BL_PROTO_REV 0x2

/*
 * TODO: Maximum block transfer in reply
 * Handshake: BL_PROTO_CMD_HANDSHAKE
 *   Req: CmdID, EOM, CRC8
 *   Rep: Ver, Rev,<BoardID1,BoardID2,BoardID3,BoardID4>,
 *        <AppSz1,AppSz2,AppSz3,AppSz4>, EOM, CRC8
 *
 * Erase: BL_PROTO_CMD_ERASE
 *   Req: CmdId, Sector, EOM, CRC8
 *   Rep: Status, EOM, CRC8
 *
 * Flash start: BL_PROTO_CMD_FLASH
 *   Req: CmdId, <A1,A2,A3,A4>, <S1,S2,S3,S4>, EOM, CRC8
 *   Rep: Status, EOM, CRC8
 *
 * Flash data: BL_PROTO_CMD_FLASH_DATA
 *   Req: CmdId, Size, <Data>, CRC8 (CRC8 is calculated excluding <Data>)
 *   Rep: Status, EOM, CRC8
 *
 * Data CRC: BL_PROTO_CMD_DATA_CRC
 *   Req: CmdId, <SA1,SA2,SA3,SA4>, <EA1,EA2,EA3,EA4>, EOM, CRC8
 *   Rep: Status, <CRC1,CRC2,CRC3,CRC4>, EOM, CRC8
 *
 * Data CRC: BL_PROTO_CMD_BOOT
 *      Req: CmdId, <A1,A2,A3,A4>, EOM, CRC8
 *      Rep: -none-
 *
 * End Of Session: BL_PROTO_CMD_EOS
 *   Req: CmdId, EOM, CRC8
 *   Rep: Status, EOM, CRC8
 */

#define BL_PROTO_CMD_HANDSHAKE  0x0A
#define BL_PROTO_CMD_ERASE      0xB1
#define BL_PROTO_CMD_FLASH      0xB2
#define BL_PROTO_CMD_FLASH_DATA 0xB3
#define BL_PROTO_CMD_EOS        0x0E
#define BL_PROTO_CMD_DATA_CRC   0xEF
#define BL_PROTO_CMD_BOOT       0x0F

#define BL_PROTO_EOM 0xC0

#define BL_PROTO_STATUS_OK      0xA0
#define BL_PROTO_STATUS_IOERR   0x0A
#define BL_PROTO_STATUS_ARGERR  0x1
#define BL_PROTO_STATUS_CRCERR  0x2
#define BL_PROTO_STATUS_READERR 0x3

#endif /* __BL_PROTOCOL_H */
