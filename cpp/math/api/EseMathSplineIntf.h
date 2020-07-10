#ifndef _ese_math_spline_intf_h_
#define _ese_math_spline_intf_h_

// Forward decl
class EseStreamIntf;
class EseMathSpline;
class EseCrcIntf;

/// Math spline interface abstraction
///
class ESE_ABSTRACT EseMathSplineIntf
{
public:
  /// Dynamic interfaced object deallocation
  virtual void destroy() ESE_NOTHROW = 0;
  
  /// Return true if this spline data match other data
  virtual bool isEqualTo(const EseMathSplineIntf& other) const ESE_NOTHROW = 0;

  /// Return true if spline nodes were not set-up
  virtual bool isEmpty() const ESE_NOTHROW = 0;
  
  /// Return maximum allowed spline nodes count
  virtual size_t maxNodesCountGet() const ESE_NOTHROW = 0;
  
  /// Get current spline nodes count
  virtual size_t nodesCountGet() const ESE_NOTHROW = 0;

  /// Set current spline nodes count
  virtual void nodesCountSet(size_t cnt) ESE_NOTHROW = 0;

  /// Assign nodes buffer to the spline interface object
  virtual void assign(const EseMathSplineIntf& other) ESE_NOTHROW = 0;
  virtual void assign(const EseMathSpline& other) ESE_NOTHROW = 0;
  
  /// Set spline node
  virtual void nodeSet(size_t idx, float x, float a, float b, float c, float d) ESE_NOTHROW = 0;
  
  /// Get spline node. If node successfully received, a true is returned, false otherwise.
  /// If false is returned, output parameters should be considered invalid
  ///
  virtual bool nodeGet(size_t idx, float& x, float& a, float& b, float& c, float& d) const ESE_NOTHROW = 0;
  
  /// Cleanup nodes buffer, if needed.
  /// Spline becomes isEmpty after call to this method.
  ///
  virtual void cleanup() ESE_NOTHROW = 0;
  
  /// Calculate spline value at random x. 
  /// If spline is empty - quiet NaN is returned
  /// @param x      [in]  spline calculation argument
  /// @param valid  [out] calculation validity flag. Set to false, if
  ///                     calculation result is invalid (qNaN), true otherwise.
  /// @return       Calculation result, or quiet NaN
  ///
  virtual float valueGet(float x, bool& valid) const ESE_NOTHROW = 0;

  /// Access raw spline object
  virtual const EseMathSpline& splineRawGet() const ESE_NOTHROW = 0;
  
  /// Update Spline CRC to given CRC interface object
  virtual void crcUpdate(EseCrcIntf& icrc) const ESE_NOTHROW = 0;

#ifdef ESE_USE_SPLINE_SERIALIZATION
  /// Serializable implementation
  ///

# if defined(ESE_USE_SPLINE_SERIALIZATION_READ) && 0 != ESE_USE_SPLINE_SERIALIZATION_READ
  /// Read spline object from stream, optionally limiting maximum count of nodes to be read
  /// @param in     [in] stream object, spline data to be read from
  /// @param maxNodes [optional, in] maximum count of spline nodes to be read from stream
  /// @return       true, on read success, false otherwise.
  ///
  virtual bool readFrom(EseStreamIntf& in) ESE_NOTHROW = 0;
# endif

# if defined(ESE_USE_SPLINE_SERIALIZATION_WRITE) && 0 != ESE_USE_SPLINE_SERIALIZATION_WRITE
  /// Write spline object to stream
  /// @param out    [in] stream object, spline data to be written to
  /// @return       true, on write success, false otherwise.
  ///
  virtual bool writeTo(EseStreamIntf& out) const ESE_NOTHROW = 0;
  
  /// Write spline object's nodes block to stream
  /// @param out    [in] stream object, spline nodes data to be written to
  /// @return       true, on write success, false otherwise.
  ///
  virtual bool writeNodesTo(EseStreamIntf& out) const ESE_NOTHROW = 0;
# endif
#endif

  /// Raw data assignment with optional ownership
  virtual void assign(const uint8_t* data, size_t dataLen, bool owning = true) ESE_NOTHROW = 0;
};

#endif //< _ese_math_spline_intf_h_
