#ifndef __BL_PROTOCOL_H
#define __BL_PROTOCOL_H

#define BL_PROTO_VER 0x1
#define BL_PROTO_REV 0x1

/*
 * TODO: Maximum block transfer in reply
 * Handshake: BL_PROTO_CMD_HANDSHAKE
 *   Req: CmdID, EOM, CRC8
 *   Rep: Ver, Rev, BoardID_Byte1, BoardID_Byte2, BoardID_Byte3,
 *        BoardID_Byte4, FW_Size_Area, EOM, CRC8
 *
 * Erase: BL_PROTO_CMD_ERASE
 *   Req: CmdId, Sector, EOM, CRC8
 *   Rep: Status, EOM, CRC8
 *
 * Flash start: BL_PROTO_CMD_FLASH
 *   Req_: CmdId, Addr, CRC8
 *   Rep: Status, EOM, CRC8
 *
 * Flash data: BL_PROTO_CMD_FLASH_DATA
 *   Req: CmdId, Size, CRC8
 *   Rep: Status, EOM, CRC8
 *   Req: Data
 *
 * Data CRC: BL_PROTO_CMD_DATA_CRC
 *   Req: CmdId, SA1,SA2,SA3,SA4, EA1,EA2,EA3,EA4, EOM, CRC8
 *   Rep: CRC1,CRC2,CRC3,CRC4, EOM, CRC8
 *
 * End Of Session: BL_PROTO_CMD_EOS
 *   Req: CmdId, EOM, CRC8
 *   Rep: Status, EOM, CRC8
 */

#define BL_PROTO_CMD_HANDSHAKE  0xA
#define BL_PROTO_CMD_ERASE      0xB
#define BL_PROTO_CMD_FLASH      0xC
#define BL_PROTO_CMD_FLASH_DATA 0xD
#define BL_PROTO_CMD_EOS        0xE
#define BL_PROTO_CMD_DATA_CRC   0xF

#define BL_PROTO_EOM 0xC0

#define BL_PROTO_STATUS_OK     0xA0
#define BL_PROTO_STATUS_RWERR  0x0A
#define BL_PROTO_STATUS_ARGERR 0x1
#define BL_PROTO_STATUS_CRCERR 0x2

#endif /* __BL_PROTOCOL_H */
