UPDATE	TB_UNIQUE_DEV_NODE
SET		DESCRIPTION = 'light sensor'
		,DEV_TYPE = 'SENSOR'
		,LINK1 = 'http://www.liteon-semi.com/_en/03_isbu/02_about.php?ID=8&SID=6'
WHERE	DEV_NODE = '/dev/al3006_pls'
