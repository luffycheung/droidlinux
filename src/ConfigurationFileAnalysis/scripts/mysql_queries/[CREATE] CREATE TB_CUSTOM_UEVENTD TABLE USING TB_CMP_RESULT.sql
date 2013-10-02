CREATE	TABLE TB_CUSTOM_UEVENTD AS (
SELECT	A.FW_FILE_NAME AS FW_FILE_NAME
		,B.OS_VERSION
		,A.DEV_NODE
		,A.C_PERMISSION AS PERMISSION
		,A.C_USER AS USER
		,A.C_GROUP AS "GROUP"
FROM	TB_UEVENTD_CMP_RESULT AS A
		LEFT OUTER JOIN
		(
		SELECT	*
		FROM	TB_RAW_PHONE_FW
		WHERE	LEFT(OS_VERSION,1) = 4
		) AS B
		ON (A.FW_FILE_NAME = LEFT(B.FILE_NAME,LENGTH(B.FILE_NAME)-4))
WHERE	length(C_USER) <> 0
)
;

SELECT	*
FROM	TB_NEXUS_UEVENTD
WHERE	PERMISSION = '0666';



SELECT	*
FROM	TB_CUSTOM_UEVENTD
WHERE	OS_VERSION = 
;