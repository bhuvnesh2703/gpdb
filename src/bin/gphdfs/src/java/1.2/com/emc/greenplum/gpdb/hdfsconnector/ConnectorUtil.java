/*-------------------------------------------------------------------------
 *
 * ConnectorUtil
 *
 * Copyright (c) 2011 EMC Corporation All Rights Reserved
 *
 * This software is protected, without limitation, by copyright law
 * and international treaties. Use of this software and the intellectual
 * property contained therein is expressly limited to the terms and
 * conditions of the License Agreement under which it is provided by
 * or on behalf of EMC.
 *
 *-------------------------------------------------------------------------
 */

package com.emc.greenplum.gpdb.hdfsconnector;

import com.emc.greenplum.gpdb.hadoop.io.GPDBWritable;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.security.SecurityUtil;

import java.net.URI;
import java.io.IOException;


/**
 * This is a common utility for the GPDB side Hadoop connector. Routines common to
 * HDFSReader and HDFSWriter would appear here.
 * Therefore, most of the routine should be protected.
 * @author achoi
 *
 */
public class ConnectorUtil
{
	/**
	 * Helper routine to translate the External table URI to the actual
	 * Hadoop fs.default.name and set it in the conf.
	 * MapR's URI starts with maprfs:///
	 * Hadoop's URI starts with hdfs://
	 * 
	 * @param conf the configuration
	 * @param inputURI the external table URI
	 * @param connVer the Hadoop Connector version
	 */
	protected static void setHadoopFSURI(Configuration conf, URI inputURI, String connVer)
	{
		/**
		 * Parse the URI and reconstruct the destination URI
		 * Scheme could be hdfs or maprfs
		 * 
		 * NOTE: Unless the version is MR, we're going to use ordinary
		 * hdfs://. 
		 */
		if (connVer.startsWith("gpmr")) {
			conf.set("fs.maprfs.impl", "com.mapr.fs.MapRFileSystem");
			conf.set("fs.defaultFS", "maprfs:///" + inputURI.getHost());
		} else {
			String uri = inputURI.getHost();
			if (inputURI.getPort() != -1) {
				uri += ":" + inputURI.getPort();
			}

			conf.set("fs.defaultFS", "hdfs://" + uri);
		}
	}

	protected static final String HADOOP_SECURITY_USER_KEYTAB_FILE = 
		"com.emc.greenplum.gpdb.hdfsconnector.security.user.keytab.file";
	protected static final String HADOOP_SECURITY_USERNAME = 
		"com.emc.greenplum.gpdb.hdfsconnector.security.user.name";

	/**
	 * Helper routine to login to secure Hadoop. If it's not configured to use
	 * security (in the core-site.xml), then UserGroupInformation.loginUserFromKeytab
	 * would do nothing.
	 * 
	 * @param conf the configuration
	 */
	protected static void loginSecureHadoop(Configuration conf) throws IOException
	{
	    String principalName  = conf.get(HADOOP_SECURITY_USERNAME);
	    String keytabFilename = conf.get(HADOOP_SECURITY_USER_KEYTAB_FILE);
	    SecurityUtil.login(conf,
	    		HADOOP_SECURITY_USER_KEYTAB_FILE,
	    		HADOOP_SECURITY_USERNAME);
	}
}
