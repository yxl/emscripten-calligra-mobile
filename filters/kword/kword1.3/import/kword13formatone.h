
#ifndef KWORD_1_3_FORMAT_ONE
#define KWORD_1_3_FORMAT_ONE

class QTextStream;

#include <qstring.h>
#include <qmap.h>

/**
 * Contains the data of the \<FORMAT id="1"\> children
 * also the grand-children of \<STYLE\> and \<LAYOUT\>
 */
class KWord13FormatOneData
{
public:
    KWord13FormatOneData( void );
    ~KWord13FormatOneData( void );
    
public:
    void xmldump( QTextStream& iostream );
    
    /**
     * @brief Get a key representating the properties
     *
     * This key helps to categorize the automatic styles
     */
    QString key( void ) const;

    QString getProperty( const QString& name ) const;

public:
    QMap<QString,QString> m_properties;
public: // OASIS-specific
    QString m_autoStyleName; ///< Name of the OASIS automatic style
};

#endif // KWORD_1_3_FORMAT_ONE
