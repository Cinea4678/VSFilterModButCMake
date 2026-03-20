#pragma once
// Cross-platform replacements for MFC/ATL geometric types and collections

#ifndef _WIN32

#include "compat_windows_types.h"
#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <algorithm>
#include <cstring>
#include <cstdarg>
#include <cwchar>
#include <mutex>

// ============================================================
// CPoint - replacement for MFC CPoint
// ============================================================
class CPoint : public POINT {
public:
    CPoint() { x = 0; y = 0; }
    CPoint(int _x, int _y) { x = _x; y = _y; }
    CPoint(POINT pt) { x = pt.x; y = pt.y; }
    CPoint(SIZE sz) { x = sz.cx; y = sz.cy; }

    void Offset(int dx, int dy) { x += dx; y += dy; }
    void Offset(POINT pt) { x += pt.x; y += pt.y; }
    void SetPoint(int _x, int _y) { x = _x; y = _y; }

    bool operator==(const POINT& pt) const { return x == pt.x && y == pt.y; }
    bool operator!=(const POINT& pt) const { return !(*this == pt); }
    CPoint operator+(const POINT& pt) const { return CPoint(x + pt.x, y + pt.y); }
    CPoint operator-(const POINT& pt) const { return CPoint(x - pt.x, y - pt.y); }
    CPoint operator+(const SIZE& sz) const { return CPoint(x + sz.cx, y + sz.cy); }
    CPoint operator-(const SIZE& sz) const { return CPoint(x - sz.cx, y - sz.cy); }
    CPoint& operator+=(const POINT& pt) { x += pt.x; y += pt.y; return *this; }
    CPoint& operator-=(const POINT& pt) { x -= pt.x; y -= pt.y; return *this; }
    CPoint operator-() const { return CPoint(-x, -y); }
};

// ============================================================
// CSize - replacement for MFC CSize
// ============================================================
class CSize : public SIZE {
public:
    CSize() { cx = 0; cy = 0; }
    CSize(int _cx, int _cy) { cx = _cx; cy = _cy; }
    CSize(SIZE sz) { cx = sz.cx; cy = sz.cy; }
    CSize(POINT pt) { cx = pt.x; cy = pt.y; }

    bool operator==(const SIZE& sz) const { return cx == sz.cx && cy == sz.cy; }
    bool operator!=(const SIZE& sz) const { return !(*this == sz); }
    CSize operator+(const SIZE& sz) const { return CSize(cx + sz.cx, cy + sz.cy); }
    CSize operator-(const SIZE& sz) const { return CSize(cx - sz.cx, cy - sz.cy); }
    CSize& operator+=(const SIZE& sz) { cx += sz.cx; cy += sz.cy; return *this; }
    CSize& operator-=(const SIZE& sz) { cx -= sz.cx; cy -= sz.cy; return *this; }
    CSize operator-() const { return CSize(-cx, -cy); }
};

// ============================================================
// CRect - replacement for MFC CRect
// ============================================================
class CRect : public RECT {
public:
    CRect() { left = top = right = bottom = 0; }
    CRect(int l, int t, int r, int b) { left = l; top = t; right = r; bottom = b; }
    CRect(const RECT& rc) { left = rc.left; top = rc.top; right = rc.right; bottom = rc.bottom; }
    CRect(POINT pt, SIZE sz) { left = pt.x; top = pt.y; right = pt.x + sz.cx; bottom = pt.y + sz.cy; }
    CRect(POINT topLeft, POINT bottomRight) { left = topLeft.x; top = topLeft.y; right = bottomRight.x; bottom = bottomRight.y; }

    int Width() const { return right - left; }
    int Height() const { return bottom - top; }
    CSize Size() const { return CSize(Width(), Height()); }
    CPoint TopLeft() const { return CPoint(left, top); }
    CPoint BottomRight() const { return CPoint(right, bottom); }
    CPoint CenterPoint() const { return CPoint((left + right) / 2, (top + bottom) / 2); }

    bool IsRectEmpty() const { return left >= right || top >= bottom; }
    bool IsRectNull() const { return left == 0 && top == 0 && right == 0 && bottom == 0; }
    bool PtInRect(POINT pt) const { return pt.x >= left && pt.x < right && pt.y >= top && pt.y < bottom; }

    void SetRect(int l, int t, int r, int b) { left = l; top = t; right = r; bottom = b; }
    void SetRectEmpty() { left = top = right = bottom = 0; }
    void InflateRect(int dx, int dy) { left -= dx; top -= dy; right += dx; bottom += dy; }
    void InflateRect(int l, int t, int r, int b) { left -= l; top -= t; right += r; bottom += b; }
    void InflateRect(SIZE sz) { InflateRect(sz.cx, sz.cy); }
    void DeflateRect(int dx, int dy) { InflateRect(-dx, -dy); }
    void DeflateRect(int l, int t, int r, int b) { InflateRect(-l, -t, -r, -b); }
    void DeflateRect(SIZE sz) { DeflateRect(sz.cx, sz.cy); }
    void OffsetRect(int dx, int dy) { left += dx; top += dy; right += dx; bottom += dy; }
    void OffsetRect(POINT pt) { OffsetRect(pt.x, pt.y); }
    void OffsetRect(SIZE sz) { OffsetRect(sz.cx, sz.cy); }
    void NormalizeRect() {
        if (left > right) std::swap(left, right);
        if (top > bottom) std::swap(top, bottom);
    }
    void MoveToX(int x) { right = x + Width(); left = x; }
    void MoveToY(int y) { bottom = y + Height(); top = y; }
    void MoveToXY(int x, int y) { MoveToX(x); MoveToY(y); }
    void MoveToXY(POINT pt) { MoveToXY(pt.x, pt.y); }

    bool IntersectRect(const RECT* r1, const RECT* r2) {
        left = std::max(r1->left, r2->left);
        top = std::max(r1->top, r2->top);
        right = std::min(r1->right, r2->right);
        bottom = std::min(r1->bottom, r2->bottom);
        if (left >= right || top >= bottom) { SetRectEmpty(); return false; }
        return true;
    }
    bool UnionRect(const RECT* r1, const RECT* r2) {
        left = std::min(r1->left, r2->left);
        top = std::min(r1->top, r2->top);
        right = std::max(r1->right, r2->right);
        bottom = std::max(r1->bottom, r2->bottom);
        return !IsRectEmpty();
    }

    bool operator==(const RECT& rc) const { return left == rc.left && top == rc.top && right == rc.right && bottom == rc.bottom; }
    bool operator!=(const RECT& rc) const { return !(*this == rc); }
    CRect operator&(const RECT& rc) const { CRect r; r.IntersectRect(this, &rc); return r; }
    CRect operator|(const RECT& rc) const { CRect r; r.UnionRect(this, &rc); return r; }
    CRect& operator&=(const RECT& rc) { IntersectRect(this, &rc); return *this; }
    CRect& operator|=(const RECT& rc) { UnionRect(this, &rc); return *this; }
    CRect operator+(POINT pt) const { CRect r(*this); r.OffsetRect(pt); return r; }
    CRect operator-(POINT pt) const { CRect r(*this); r.OffsetRect(-pt.x, -pt.y); return r; }

    operator RECT*() { return this; }
    operator const RECT*() const { return this; }
};

// ============================================================
// CString - replacement for MFC CString (wide version)
// ============================================================
class CStringW {
    std::wstring m_str;
public:
    CStringW() {}
    CStringW(const wchar_t* s) : m_str(s ? s : L"") {}
    CStringW(const wchar_t* s, int len) : m_str(s, len) {}
    CStringW(const char* s) {
        if (s) {
            // Simple ASCII→wchar conversion
            while (*s) m_str += (wchar_t)(unsigned char)*s++;
        }
    }
    CStringW(const std::wstring& s) : m_str(s) {}
    CStringW(wchar_t ch, int repeat = 1) : m_str(repeat, ch) {}

    // Conversions
    operator const wchar_t*() const { return m_str.c_str(); }
    operator LPCWSTR() const { return m_str.c_str(); }

    const wchar_t* GetString() const { return m_str.c_str(); }
    int GetLength() const { return (int)m_str.length(); }
    bool IsEmpty() const { return m_str.empty(); }
    void Empty() { m_str.clear(); }

    wchar_t GetAt(int i) const { return m_str[i]; }
    void SetAt(int i, wchar_t ch) { m_str[i] = ch; }
    wchar_t operator[](int i) const { return m_str[i]; }

    // Modification
    CStringW& operator=(const wchar_t* s) { m_str = s ? s : L""; return *this; }
    CStringW& operator=(const CStringW& s) { m_str = s.m_str; return *this; }
    CStringW& operator=(wchar_t ch) { m_str = std::wstring(1, ch); return *this; }
    CStringW& operator+=(const wchar_t* s) { if (s) m_str += s; return *this; }
    CStringW& operator+=(const CStringW& s) { m_str += s.m_str; return *this; }
    CStringW& operator+=(wchar_t ch) { m_str += ch; return *this; }

    friend CStringW operator+(const CStringW& a, const CStringW& b) { return CStringW(a.m_str + b.m_str); }
    friend CStringW operator+(const CStringW& a, const wchar_t* b) { return CStringW(a.m_str + (b ? b : L"")); }
    friend CStringW operator+(const wchar_t* a, const CStringW& b) { return CStringW((a ? a : L"") + b.m_str); }
    friend CStringW operator+(const CStringW& a, wchar_t ch) { return CStringW(a.m_str + std::wstring(1, ch)); }

    // Comparison
    int Compare(const wchar_t* s) const { return m_str.compare(s ? s : L""); }
    int CompareNoCase(const wchar_t* s) const {
        std::wstring a = m_str, b = s ? s : L"";
        std::transform(a.begin(), a.end(), a.begin(), ::towlower);
        std::transform(b.begin(), b.end(), b.begin(), ::towlower);
        return a.compare(b);
    }

    bool operator==(const wchar_t* s) const { return m_str == (s ? s : L""); }
    bool operator==(const CStringW& s) const { return m_str == s.m_str; }
    bool operator!=(const wchar_t* s) const { return !(*this == s); }
    bool operator!=(const CStringW& s) const { return !(*this == s); }
    bool operator<(const CStringW& s) const { return m_str < s.m_str; }
    bool operator>(const CStringW& s) const { return m_str > s.m_str; }
    bool operator<=(const CStringW& s) const { return m_str <= s.m_str; }
    bool operator>=(const CStringW& s) const { return m_str >= s.m_str; }

    // Substring
    CStringW Mid(int first) const { return CStringW(m_str.substr(first)); }
    CStringW Mid(int first, int count) const { return CStringW(m_str.substr(first, count)); }
    CStringW Left(int count) const { return CStringW(m_str.substr(0, count)); }
    CStringW Right(int count) const {
        if (count >= (int)m_str.length()) return *this;
        return CStringW(m_str.substr(m_str.length() - count));
    }

    // Search
    int Find(wchar_t ch, int start = 0) const {
        auto pos = m_str.find(ch, start);
        return pos == std::wstring::npos ? -1 : (int)pos;
    }
    int Find(const wchar_t* s, int start = 0) const {
        auto pos = m_str.find(s, start);
        return pos == std::wstring::npos ? -1 : (int)pos;
    }
    int ReverseFind(wchar_t ch) const {
        auto pos = m_str.rfind(ch);
        return pos == std::wstring::npos ? -1 : (int)pos;
    }
    int FindOneOf(const wchar_t* chars) const {
        auto pos = m_str.find_first_of(chars);
        return pos == std::wstring::npos ? -1 : (int)pos;
    }

    // Modification
    CStringW& MakeLower() {
        std::transform(m_str.begin(), m_str.end(), m_str.begin(), ::towlower);
        return *this;
    }
    CStringW& MakeUpper() {
        std::transform(m_str.begin(), m_str.end(), m_str.begin(), ::towupper);
        return *this;
    }
    CStringW& Trim() { TrimLeft(); TrimRight(); return *this; }
    CStringW& TrimLeft() {
        m_str.erase(0, m_str.find_first_not_of(L" \t\r\n"));
        return *this;
    }
    CStringW& TrimRight() {
        auto pos = m_str.find_last_not_of(L" \t\r\n");
        if (pos != std::wstring::npos) m_str.erase(pos + 1);
        else m_str.clear();
        return *this;
    }
    CStringW& TrimLeft(wchar_t ch) {
        m_str.erase(0, m_str.find_first_not_of(ch));
        return *this;
    }
    CStringW& TrimRight(wchar_t ch) {
        auto pos = m_str.find_last_not_of(ch);
        if (pos != std::wstring::npos) m_str.erase(pos + 1);
        else m_str.clear();
        return *this;
    }
    CStringW& Trim(wchar_t ch) { TrimLeft(ch); TrimRight(ch); return *this; }

    int Replace(const wchar_t* old_str, const wchar_t* new_str) {
        int count = 0;
        size_t old_len = wcslen(old_str);
        size_t new_len = wcslen(new_str);
        size_t pos = 0;
        while ((pos = m_str.find(old_str, pos)) != std::wstring::npos) {
            m_str.replace(pos, old_len, new_str);
            pos += new_len;
            count++;
        }
        return count;
    }
    int Replace(wchar_t old_ch, wchar_t new_ch) {
        int count = 0;
        for (auto& ch : m_str) if (ch == old_ch) { ch = new_ch; count++; }
        return count;
    }
    int Remove(wchar_t ch) {
        int count = 0;
        std::wstring result;
        for (auto c : m_str) {
            if (c == ch) count++;
            else result += c;
        }
        m_str = result;
        return count;
    }
    int Insert(int index, wchar_t ch) { m_str.insert(index, 1, ch); return (int)m_str.length(); }
    int Insert(int index, const wchar_t* s) { m_str.insert(index, s); return (int)m_str.length(); }
    int Delete(int index, int count = 1) { m_str.erase(index, count); return (int)m_str.length(); }

    // Format
    void __attribute__((format(wprintf, 2, 0))) FormatV(const wchar_t* fmt, va_list args) {
        va_list args_copy;
        va_copy(args_copy, args);
        int len = vswprintf(nullptr, 0, fmt, args_copy);
        va_end(args_copy);
        if (len < 0) { m_str.clear(); return; }
        m_str.resize(len);
        vswprintf(&m_str[0], len + 1, fmt, args);
    }
    void Format(const wchar_t* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        FormatV(fmt, args);
        va_end(args);
    }
    void AppendFormat(const wchar_t* fmt, ...) {
        CStringW tmp;
        va_list args;
        va_start(args, fmt);
        tmp.FormatV(fmt, args);
        va_end(args);
        m_str += tmp.m_str;
    }

    // Buffer access
    wchar_t* GetBuffer(int minLen = 0) {
        if (minLen > 0 && (int)m_str.size() < minLen)
            m_str.resize(minLen);
        return &m_str[0];
    }
    void ReleaseBuffer(int newLen = -1) {
        if (newLen >= 0) m_str.resize(newLen);
        else {
            auto pos = m_str.find(L'\0');
            if (pos != std::wstring::npos) m_str.resize(pos);
        }
    }
    int GetAllocLength() const { return (int)m_str.capacity(); }

    // std::wstring interop
    const std::wstring& GetStdString() const { return m_str; }

private:
    CStringW(const std::wstring& s, int) : m_str(s) {} // tag constructor
};

// CStringA - ANSI version
class CStringA {
    std::string m_str;
public:
    CStringA() {}
    CStringA(const char* s) : m_str(s ? s : "") {}
    CStringA(const char* s, int len) : m_str(s, len) {}
    CStringA(const wchar_t* s) {
        if (s) {
            // Simple wchar→ASCII conversion
            while (*s) m_str += (char)(*s++ & 0x7F);
        }
    }
    CStringA(const CStringW& s) {
        const wchar_t* w = (const wchar_t*)s;
        while (*w) m_str += (char)(*w++ & 0x7F);
    }
    CStringA(char ch, int repeat = 1) : m_str(repeat, ch) {}

    operator const char*() const { return m_str.c_str(); }
    const char* GetString() const { return m_str.c_str(); }
    int GetLength() const { return (int)m_str.length(); }
    bool IsEmpty() const { return m_str.empty(); }
    void Empty() { m_str.clear(); }
    char GetAt(int i) const { return m_str[i]; }
    char operator[](int i) const { return m_str[i]; }

    CStringA& operator=(const char* s) { m_str = s ? s : ""; return *this; }
    CStringA& operator+=(const char* s) { if (s) m_str += s; return *this; }
    CStringA& operator+=(const CStringA& s) { m_str += s.m_str; return *this; }

    friend CStringA operator+(const CStringA& a, const CStringA& b) { CStringA r; r.m_str = a.m_str + b.m_str; return r; }

    int Compare(const char* s) const { return m_str.compare(s ? s : ""); }
    bool operator==(const char* s) const { return m_str == (s ? s : ""); }
    bool operator!=(const char* s) const { return !(*this == s); }

    int Find(char ch, int start = 0) const {
        auto pos = m_str.find(ch, start);
        return pos == std::string::npos ? -1 : (int)pos;
    }

    int Replace(const char* old_str, const char* new_str) {
        int count = 0;
        size_t old_len = strlen(old_str);
        size_t new_len = strlen(new_str);
        size_t pos = 0;
        while ((pos = m_str.find(old_str, pos)) != std::string::npos) {
            m_str.replace(pos, old_len, new_str);
            pos += new_len;
            count++;
        }
        return count;
    }

    void Format(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        char buf[4096];
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        m_str = buf;
    }

    char* GetBuffer(int minLen = 0) {
        if (minLen > 0 && (int)m_str.size() < minLen) m_str.resize(minLen);
        return &m_str[0];
    }
    void ReleaseBuffer(int newLen = -1) {
        if (newLen >= 0) m_str.resize(newLen);
        else {
            auto pos = m_str.find('\0');
            if (pos != std::string::npos) m_str.resize(pos);
        }
    }
};

// CString is the wide version by default (matches MFC UNICODE build)
typedef CStringW CString;

// ============================================================
// ATL Collection replacements
// ============================================================

template<typename T>
class CAtlArray : public std::vector<T> {
public:
    using std::vector<T>::vector;

    size_t GetCount() const { return this->size(); }
    bool IsEmpty() const { return this->empty(); }
    size_t Add(const T& val) { this->push_back(val); return this->size() - 1; }
    void RemoveAll() { this->clear(); }
    void RemoveAt(size_t index) { this->erase(this->begin() + index); }
    void SetCount(size_t count) { this->resize(count); }
    T* GetData() { return this->data(); }
    const T* GetData() const { return this->data(); }
    void InsertAt(size_t index, const T& val) { this->insert(this->begin() + index, val); }
    const T& GetAt(size_t index) const { return (*this)[index]; }
    T& GetAt(size_t index) { return (*this)[index]; }
    void SetAt(size_t index, const T& val) { (*this)[index] = val; }
    void Copy(const CAtlArray& other) { *this = other; }
    void Append(const CAtlArray& other) { this->insert(this->end(), other.begin(), other.end()); }
};

template<typename T>
class CAtlList {
    std::list<T> m_list;
public:
    typedef typename std::list<T>::iterator POSITION_ITER;

    // Position is represented as a pointer to node
    // We use a simple index-based approach for compatibility
    struct Position {
        typename std::list<T>::iterator it;
        bool valid;
        Position() : valid(false) {}
        Position(typename std::list<T>::iterator i) : it(i), valid(true) {}
        operator bool() const { return valid; }
    };
    typedef Position* POSITION;

    size_t GetCount() const { return m_list.size(); }
    bool IsEmpty() const { return m_list.empty(); }
    void RemoveAll() { m_list.clear(); m_positions.clear(); }

    // Note: MFC CAtlList uses POSITION as opaque pointer
    // We simplify by using index. This won't be perfectly compatible
    // but covers common usage patterns.
    T& GetHead() { return m_list.front(); }
    const T& GetHead() const { return m_list.front(); }
    T& GetTail() { return m_list.back(); }
    const T& GetTail() const { return m_list.back(); }

    void AddTail(const T& val) { m_list.push_back(val); }
    void AddHead(const T& val) { m_list.push_front(val); }
    T RemoveHead() { T val = m_list.front(); m_list.pop_front(); return val; }
    T RemoveTail() { T val = m_list.back(); m_list.pop_back(); return val; }

    // Simplified iteration - users should migrate to range-for
    auto begin() { return m_list.begin(); }
    auto end() { return m_list.end(); }
    auto begin() const { return m_list.begin(); }
    auto end() const { return m_list.end(); }

private:
    std::vector<Position> m_positions;
};

template<typename K, typename V>
class CAtlMap {
    std::map<K, V> m_map;
public:
    size_t GetCount() const { return m_map.size(); }
    bool IsEmpty() const { return m_map.empty(); }
    void RemoveAll() { m_map.clear(); }

    V& operator[](const K& key) { return m_map[key]; }

    bool Lookup(const K& key, V& val) const {
        auto it = m_map.find(key);
        if (it != m_map.end()) { val = it->second; return true; }
        return false;
    }

    void SetAt(const K& key, const V& val) { m_map[key] = val; }
    bool RemoveKey(const K& key) { return m_map.erase(key) > 0; }
};

// ============================================================
// Smart pointer replacements
// ============================================================

template<typename T>
class CAutoPtr {
    T* m_ptr;
public:
    CAutoPtr() : m_ptr(nullptr) {}
    explicit CAutoPtr(T* p) : m_ptr(p) {}
    CAutoPtr(CAutoPtr&& other) : m_ptr(other.m_ptr) { other.m_ptr = nullptr; }
    ~CAutoPtr() { delete m_ptr; }

    CAutoPtr& operator=(CAutoPtr&& other) {
        if (this != &other) { delete m_ptr; m_ptr = other.m_ptr; other.m_ptr = nullptr; }
        return *this;
    }

    void Attach(T* p) { delete m_ptr; m_ptr = p; }
    T* Detach() { T* p = m_ptr; m_ptr = nullptr; return p; }
    void Free() { delete m_ptr; m_ptr = nullptr; }

    T* operator->() const { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    operator T*() const { return m_ptr; }
    operator bool() const { return m_ptr != nullptr; }

    CAutoPtr(const CAutoPtr&) = delete;
    CAutoPtr& operator=(const CAutoPtr&) = delete;
};

template<typename T>
class CAutoPtrList {
    std::list<CAutoPtr<T>> m_list;
public:
    size_t GetCount() const { return m_list.size(); }
    bool IsEmpty() const { return m_list.empty(); }
    void RemoveAll() { m_list.clear(); }
    void AddTail(CAutoPtr<T>& p) { m_list.push_back(std::move(p)); }

    auto begin() { return m_list.begin(); }
    auto end() { return m_list.end(); }
};

// ============================================================
// COM-like smart pointers (simplified)
// ============================================================

// IUnknown base interface
class IUnknown {
public:
    virtual HRESULT QueryInterface(REFIID riid, void** ppvObject) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual ~IUnknown() {}
};

template<typename T>
class CComPtr {
    T* m_ptr;
public:
    CComPtr() : m_ptr(nullptr) {}
    CComPtr(T* p) : m_ptr(p) { if (m_ptr) m_ptr->AddRef(); }
    CComPtr(const CComPtr& other) : m_ptr(other.m_ptr) { if (m_ptr) m_ptr->AddRef(); }
    ~CComPtr() { if (m_ptr) m_ptr->Release(); }

    CComPtr& operator=(T* p) {
        if (m_ptr != p) {
            if (m_ptr) m_ptr->Release();
            m_ptr = p;
            if (m_ptr) m_ptr->AddRef();
        }
        return *this;
    }
    CComPtr& operator=(const CComPtr& other) { return operator=(other.m_ptr); }

    T* operator->() const { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    T** operator&() { return &m_ptr; }
    operator T*() const { return m_ptr; }
    operator bool() const { return m_ptr != nullptr; }

    void Release() { if (m_ptr) { m_ptr->Release(); m_ptr = nullptr; } }
    T* Detach() { T* p = m_ptr; m_ptr = nullptr; return p; }
    void Attach(T* p) { if (m_ptr) m_ptr->Release(); m_ptr = p; }
};

template<typename T>
class CComQIPtr : public CComPtr<T> {
public:
    CComQIPtr() {}
    CComQIPtr(T* p) : CComPtr<T>(p) {}
    template<typename U>
    CComQIPtr(U* p) {
        if (p) p->QueryInterface(__uuidof(T), (void**)&(*this));
    }
};

// ============================================================
// Synchronization primitives (replacing DirectShow base classes)
// ============================================================

class CCritSec {
    std::recursive_mutex m_mutex;
public:
    void Lock() { m_mutex.lock(); }
    void Unlock() { m_mutex.unlock(); }
    std::recursive_mutex& GetMutex() { return m_mutex; }
};

class CAutoLock {
    CCritSec* m_pLock;
public:
    CAutoLock(CCritSec* pLock) : m_pLock(pLock) { m_pLock->Lock(); }
    ~CAutoLock() { m_pLock->Unlock(); }
};

// CAMThread - simplified cross-platform version
class CAMThread {
    std::thread m_thread;
    bool m_created = false;

protected:
    virtual DWORD ThreadProc() = 0;

public:
    CAMThread() {}
    virtual ~CAMThread() {
        if (m_thread.joinable()) m_thread.join();
    }

    bool Create() {
        m_thread = std::thread([this]() { ThreadProc(); });
        m_created = true;
        return true;
    }

    DWORD CallWorker(DWORD cmd) {
        // Simplified - just signals thread to stop
        return 0;
    }

    HANDLE GetRequestHandle() { return nullptr; }
    void Reply(HRESULT hr) {}
};

// ============================================================
// File utilities
// ============================================================

struct CFileStatus {
    int64_t m_mtime;
};

inline bool CFileGetStatus(const wchar_t* fn, CFileStatus& fs) {
    // Simplified - would use stat() in real implementation
    fs.m_mtime = 0;
    return false;
}

// ============================================================
// COM interface macros (replacing DirectShow / ATL)
// ============================================================

#define STDMETHOD(method)       virtual HRESULT method
#define STDMETHOD_(type,method) virtual type method
#define STDMETHODIMP            HRESULT
#define STDMETHODIMP_(type)     type
#define PURE                    = 0
#define THIS_
#define THIS

// [uuid()] attribute - no-op on non-Windows
#define uuid(x)

// interface keyword
#define interface class

// CUnknown - base class for COM objects (simplified)
class CUnknown : public IUnknown {
    volatile ULONG m_refCount;
    CCritSec* m_pLock;
    const wchar_t* m_pName;
public:
    CUnknown(const wchar_t* pName, IUnknown* pUnk, HRESULT* phr = nullptr)
        : m_refCount(1), m_pLock(nullptr), m_pName(pName) {
        if (phr) *phr = S_OK;
    }
    CUnknown(const wchar_t* pName, IUnknown* pUnk, CCritSec* pLock, HRESULT* phr = nullptr)
        : m_refCount(1), m_pLock(pLock), m_pName(pName) {
        if (phr) *phr = S_OK;
    }
    // Also accept const char* (common in the codebase)
    CUnknown(const char* pName, IUnknown* pUnk, HRESULT* phr = nullptr)
        : m_refCount(1), m_pLock(nullptr), m_pName(nullptr) {
        if (phr) *phr = S_OK;
    }
    CUnknown(const char* pName, IUnknown* pUnk, CCritSec* pLock, HRESULT* phr = nullptr)
        : m_refCount(1), m_pLock(pLock), m_pName(nullptr) {
        if (phr) *phr = S_OK;
    }
    virtual ~CUnknown() {}

    HRESULT QueryInterface(REFIID riid, void** ppvObject) override {
        if (ppvObject) { *ppvObject = nullptr; }
        return E_NOTIMPL;
    }
    ULONG AddRef() override { return ++m_refCount; }
    ULONG Release() override {
        ULONG ref = --m_refCount;
        if (ref == 0) delete this;
        return ref;
    }

    CCritSec* GetLock() { return m_pLock; }

    // DECLARE_IUNKNOWN macro replacement
    HRESULT NonDelegatingQueryInterface(REFIID riid, void** ppv) { return E_NOTIMPL; }
};

// DECLARE_IUNKNOWN macro
#define DECLARE_IUNKNOWN \
    HRESULT QueryInterface(REFIID riid, void** ppv) override { return CUnknown::QueryInterface(riid, ppv); } \
    ULONG AddRef() override { return CUnknown::AddRef(); } \
    ULONG Release() override { return CUnknown::Release(); }

// Media type enums used in SubPicDesc
enum {
    MSP_RGB32 = 0,
    MSP_RGB24,
    MSP_RGB16,
    MSP_RGB15,
    MSP_YUY2,
    MSP_YV12,
    MSP_IYUV,
    MSP_AYUV,
    MSP_RGBA,
};

// POSITION type (MFC list iteration)
typedef void* POSITION;

// WCHAR, LCID
typedef wchar_t WCHAR;
typedef uint32_t LCID;

// IPersist (COM interface)
class IPersist : public IUnknown {
public:
    virtual HRESULT GetClassID(CLSID* pClassID) = 0;
};

// CInterfaceList (ATL collection of COM pointers)
template<typename T>
class CInterfaceList {
    std::list<CComPtr<T>> m_list;
public:
    size_t GetCount() const { return m_list.size(); }
    bool IsEmpty() const { return m_list.empty(); }
    void RemoveAll() { m_list.clear(); }
    void AddTail(T* p) { m_list.push_back(CComPtr<T>(p)); }
    void AddTail(const CComPtr<T>& p) { m_list.push_back(p); }
    CComPtr<T> RemoveHead() {
        CComPtr<T> p = m_list.front();
        m_list.pop_front();
        return p;
    }
    CComPtr<T> RemoveTail() {
        CComPtr<T> p = m_list.back();
        m_list.pop_back();
        return p;
    }
    T* GetHead() { return m_list.front(); }
    T* GetTail() { return m_list.back(); }

    POSITION GetHeadPosition() const {
        if (m_list.empty()) return nullptr;
        // Return pointer to internal node - hacky but functional
        return (POSITION)&(*m_list.begin());
    }

    auto begin() { return m_list.begin(); }
    auto end() { return m_list.end(); }
    auto begin() const { return m_list.begin(); }
    auto end() const { return m_list.end(); }
};

// __uuidof replacement (returns an empty GUID)
template<typename T>
inline REFIID __uuidof_helper() {
    static const GUID empty = {0};
    return empty;
}
#define __uuidof(T) __uuidof_helper<T>()

// NULL check
#ifndef NULL
#define NULL nullptr
#endif

// TEXTMETRIC structure
typedef struct tagTEXTMETRICW {
    LONG tmHeight;
    LONG tmAscent;
    LONG tmDescent;
    LONG tmInternalLeading;
    LONG tmExternalLeading;
    LONG tmAveCharWidth;
    LONG tmMaxCharWidth;
    LONG tmWeight;
    LONG tmOverhang;
    LONG tmDigitizedAspectX;
    LONG tmDigitizedAspectY;
    wchar_t tmFirstChar;
    wchar_t tmLastChar;
    wchar_t tmDefaultChar;
    wchar_t tmBreakChar;
    BYTE tmItalic;
    BYTE tmUnderlined;
    BYTE tmStruckOut;
    BYTE tmPitchAndFamily;
    BYTE tmCharSet;
} TEXTMETRICW, TEXTMETRIC;

// ABC structure (font metrics)
typedef struct tagABC {
    int abcA;
    UINT abcB;
    int abcC;
} ABC;

// GDI function stubs (will be replaced by FreeType in Phase 3)
#define GGO_METRICS 0
#define GGO_NATIVE  2
#define GGO_BEZIER  3
#define GGO_BITMAP  1

typedef struct tagMAT2 {
    struct { short fract; short value; } eM11;
    struct { short fract; short value; } eM12;
    struct { short fract; short value; } eM21;
    struct { short fract; short value; } eM22;
} MAT2;

typedef struct tagGLYPHMETRICS {
    UINT gmBlackBoxX;
    UINT gmBlackBoxY;
    POINT gmptGlyphOrigin;
    short gmCellIncX;
    short gmCellIncY;
} GLYPHMETRICS;

// TT_POLYGON types
#define TT_PRIM_LINE        1
#define TT_PRIM_QSPLINE     2
#define TT_PRIM_CSPLINE     3

typedef struct tagTTPOLYGONHEADER {
    DWORD cb;
    DWORD dwType;
    POINT pfxStart;
} TTPOLYGONHEADER;

typedef struct tagTTPOLYCURVE {
    WORD wType;
    WORD cpfx;
    POINT apfx[1];
} TTPOLYCURVE;

// FIXED type (16.16 fixed point)
typedef int32_t FIXED;

// Clamp helper
template<typename T>
inline T clamp(T val, T lo, T hi) {
    return val < lo ? lo : (val > hi ? hi : val);
}

#endif // !_WIN32
