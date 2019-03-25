<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
    <xsl:output method="xml" version="1.0" encoding="UTF-8" omit-xml-declaration="no" indent="yes"/>
    <xsl:strip-space elements="*"/>
    <xsl:template match="@*|node()">
      <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
      </xsl:copy>
    </xsl:template>
    <xsl:template match="@skip" >
      <xsl:attribute name="skipped">
        <xsl:value-of select="."/>
      </xsl:attribute>
    </xsl:template>
</xsl:stylesheet>
