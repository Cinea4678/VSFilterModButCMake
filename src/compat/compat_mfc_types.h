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
#include <type_traits>
#include <cwctype>
#include <mutex>
#include <thread>

// Make min/max available as unqualified names (Windows code uses min/max macros)
using std::min;
using std::max;

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

    void SetSize(int _cx, int _cy) { cx = _cx; cy = _cy; }

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
    CRect& operator+=(POINT pt) { OffsetRect(pt); return *this; }
    CRect& operator-=(POINT pt) { OffsetRect(-pt.x, -pt.y); return *this; }
    CRect& operator+=(SIZE sz) { OffsetRect(sz); return *this; }
    CRect& operator-=(SIZE sz) { OffsetRect(-sz.cx, -sz.cy); return *this; }
    CRect operator+(POINT pt) const { CRect r(*this); r.OffsetRect(pt); return r; }
    CRect operator-(POINT pt) const { CRect r(*this); r.OffsetRect(-pt.x, -pt.y); return r; }
    CRect& operator+=(const RECT& rc) { left += rc.left; top += rc.top; right += rc.right; bottom += rc.bottom; return *this; }
    CRect& operator-=(const RECT& rc) { left -= rc.left; top -= rc.top; right -= rc.right; bottom -= rc.bottom; return *this; }
    CRect operator+(const RECT& rc) const { return CRect(left + rc.left, top + rc.top, right + rc.right, bottom + rc.bottom); }
    CRect operator-(const RECT& rc) const { return CRect(left - rc.left, top - rc.top, right - rc.right, bottom - rc.bottom); }

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
    CStringW(char ch) : m_str(1, (wchar_t)(unsigned char)ch) {}
    CStringW(int ch) : m_str(1, (wchar_t)ch) {}
    // Prevent implicit construction from floating-point types
    CStringW(float) = delete;
    CStringW(double) = delete;

    // Conversions
    operator const wchar_t*() const { return m_str.c_str(); }

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
    friend CStringW operator+(wchar_t ch, const CStringW& b) { return CStringW(std::wstring(1, ch) + b.m_str); }
    friend CStringW operator+(char ch, const CStringW& b) { return CStringW(std::wstring(1, (wchar_t)ch) + b.m_str); }

    // Comparison
    int Compare(const wchar_t* s) const { return m_str.compare(s ? s : L""); }
    int CompareNoCase(const wchar_t* s) const {
        std::wstring a = m_str, b = s ? s : L"";
        std::transform(a.begin(), a.end(), a.begin(), towlower);
        std::transform(b.begin(), b.end(), b.begin(), towlower);
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
    CStringW SpanExcluding(const wchar_t* chars) const {
        auto pos = m_str.find_first_of(chars);
        if (pos == std::wstring::npos) return *this;
        return CStringW(m_str.substr(0, pos));
    }
    CStringW SpanIncluding(const wchar_t* chars) const {
        auto pos = m_str.find_first_not_of(chars);
        if (pos == std::wstring::npos) return *this;
        return CStringW(m_str.substr(0, pos));
    }
    CStringW Tokenize(const wchar_t* delimiters, int& start) const {
        if (start < 0) return CStringW();
        // Skip leading delimiters
        auto pos = m_str.find_first_not_of(delimiters, start);
        if (pos == std::wstring::npos) { start = -1; return CStringW(); }
        // Find end of token
        auto end = m_str.find_first_of(delimiters, pos);
        if (end == std::wstring::npos) {
            start = -1;
            return CStringW(m_str.substr(pos));
        }
        start = (int)end;
        return CStringW(m_str.substr(pos, end - pos));
    }

    // Modification
    CStringW& MakeLower() {
        std::transform(m_str.begin(), m_str.end(), m_str.begin(), towlower);
        return *this;
    }
    CStringW& MakeUpper() {
        std::transform(m_str.begin(), m_str.end(), m_str.begin(), towupper);
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
    CStringW& TrimLeft(const wchar_t* chars) {
        m_str.erase(0, m_str.find_first_not_of(chars));
        return *this;
    }
    CStringW& TrimRight(wchar_t ch) {
        auto pos = m_str.find_last_not_of(ch);
        if (pos != std::wstring::npos) m_str.erase(pos + 1);
        else m_str.clear();
        return *this;
    }
    CStringW& TrimRight(const wchar_t* chars) {
        auto pos = m_str.find_last_not_of(chars);
        if (pos != std::wstring::npos) m_str.erase(pos + 1);
        else m_str.clear();
        return *this;
    }
    CStringW& Trim(wchar_t ch) { TrimLeft(ch); TrimRight(ch); return *this; }
    CStringW& Trim(const wchar_t* chars) { TrimLeft(chars); TrimRight(chars); return *this; }

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
    wchar_t* GetBufferSetLength(int newLen) {
        m_str.resize(newLen);
        return &m_str[0];
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

// POSITION type (MFC list/map iteration)
typedef void* POSITION;

template<typename T>
class CAtlArray : public std::vector<T> {
public:
    using std::vector<T>::vector;

    size_t GetCount() const { return this->size(); }
    size_t GetSize() const { return this->size(); }
    bool IsEmpty() const { return this->empty(); }
    size_t Add(const T& val) { this->push_back(val); return this->size() - 1; }
    void RemoveAll() { this->clear(); }
    void RemoveAt(size_t index) { this->erase(this->begin() + index); }
    void RemoveAt(size_t index, size_t count) { this->erase(this->begin() + index, this->begin() + index + count); }
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

// MFC-compatible array types
typedef CAtlArray<UINT> CUIntArray;
typedef CAtlArray<DWORD> CDWordArray;
typedef CAtlArray<BYTE> CByteArray;

template<typename T>
class CAtlList {
    std::list<T> m_list;

    // Convert iterator to POSITION (void*) by taking address of element
    POSITION IterToPos(typename std::list<T>::iterator it) const {
        if (it == m_list.end()) return nullptr;
        return (POSITION)&(*it);
    }
    typename std::list<T>::iterator PosToIter(POSITION pos) {
        for (auto it = m_list.begin(); it != m_list.end(); ++it) {
            if ((POSITION)&(*it) == pos) return it;
        }
        return m_list.end();
    }
    typename std::list<T>::const_iterator PosToIter(POSITION pos) const {
        for (auto it = m_list.begin(); it != m_list.end(); ++it) {
            if ((POSITION)&(*it) == pos) return it;
        }
        return m_list.end();
    }

public:
    size_t GetCount() const { return m_list.size(); }
    bool IsEmpty() const { return m_list.empty(); }
    void RemoveAll() { m_list.clear(); }

    T& GetHead() { return m_list.front(); }
    const T& GetHead() const { return m_list.front(); }
    T& GetTail() { return m_list.back(); }
    const T& GetTail() const { return m_list.back(); }

    POSITION AddTail(const T& val) { m_list.push_back(val); return IterToPos(std::prev(m_list.end())); }
    POSITION AddHead(const T& val) { m_list.push_front(val); return IterToPos(m_list.begin()); }
    T RemoveHead() { T val = std::move(m_list.front()); m_list.pop_front(); return val; }
    T RemoveTail() { T val = std::move(m_list.back()); m_list.pop_back(); return val; }

    void AddTailList(const CAtlList& other) {
        for (const auto& v : other.m_list) m_list.push_back(v);
    }
    void AddTailList(const CAtlList* other) {
        if (other) AddTailList(*other);
    }

    void RemoveAt(POSITION pos) {
        auto it = PosToIter(pos);
        if (it != m_list.end()) m_list.erase(it);
    }

    POSITION GetHeadPosition() const {
        if (m_list.empty()) return nullptr;
        return (POSITION)&m_list.front();
    }
    POSITION GetTailPosition() const {
        if (m_list.empty()) return nullptr;
        return (POSITION)&m_list.back();
    }

    T& GetNext(POSITION& pos) {
        T& val = *(T*)pos;
        auto it = PosToIter(pos);
        ++it;
        pos = (it == m_list.end()) ? nullptr : (POSITION)&(*it);
        return val;
    }
    const T& GetNext(POSITION& pos) const {
        const T& val = *(const T*)pos;
        auto it = PosToIter(pos);
        ++it;
        pos = (it == m_list.end()) ? nullptr : (POSITION)&(*it);
        return val;
    }

    T& GetPrev(POSITION& pos) {
        T& val = *(T*)pos;
        auto it = PosToIter(pos);
        if (it == m_list.begin()) pos = nullptr;
        else { --it; pos = (POSITION)&(*it); }
        return val;
    }
    const T& GetPrev(POSITION& pos) const {
        const T& val = *(const T*)pos;
        auto it = PosToIter(pos);
        if (it == m_list.begin()) pos = nullptr;
        else { --it; pos = (POSITION)&(*it); }
        return val;
    }

    T& GetAt(POSITION pos) { return *(T*)pos; }
    const T& GetAt(POSITION pos) const { return *(const T*)pos; }
    void SetAt(POSITION pos, const T& val) { *(T*)pos = val; }

    POSITION FindIndex(size_t index) const {
        if (index >= m_list.size()) return nullptr;
        auto it = m_list.begin();
        std::advance(it, index);
        return (POSITION)&(*it);
    }

    POSITION Find(const T& val, POSITION startAfter = nullptr) const {
        auto it = m_list.begin();
        if (startAfter) {
            it = PosToIter(startAfter);
            if (it != m_list.end()) ++it;
        }
        for (; it != m_list.end(); ++it) {
            if (*it == val) return (POSITION)&(*it);
        }
        return nullptr;
    }

    void MoveToTail(POSITION pos) {
        auto it = const_cast<CAtlList*>(this)->PosToIter(pos);
        if (it != m_list.end()) {
            T val = std::move(*it);
            m_list.erase(it);
            m_list.push_back(std::move(val));
        }
    }

    POSITION InsertBefore(POSITION pos, const T& val) {
        auto it = PosToIter(pos);
        auto newit = m_list.insert(it, val);
        return IterToPos(newit);
    }

    POSITION InsertAfter(POSITION pos, const T& val) {
        auto it = PosToIter(pos);
        if (it != m_list.end()) ++it;
        auto newit = m_list.insert(it, val);
        return IterToPos(newit);
    }

    auto begin() { return m_list.begin(); }
    auto end() { return m_list.end(); }
    auto begin() const { return m_list.begin(); }
    auto end() const { return m_list.end(); }
};

// CStringElementTraits - no-op on non-Windows, just a tag for CAtlMap compatibility
template<typename T>
class CStringElementTraits {};

template<typename K, typename V, typename Traits = void>
class CAtlMap {
    std::map<K, V> m_map;

    // Internal pair storage for iteration
    struct IterState {
        typename std::map<K, V>::iterator it;
        const void* map_ptr;
        IterState() : map_ptr(nullptr) {}
    };

public:
    struct CPair {
        K m_key;
        V m_value;
    };

    size_t GetCount() const { return m_map.size(); }
    bool IsEmpty() const { return m_map.empty(); }
    void RemoveAll() { m_map.clear(); }

    V& operator[](const K& key) { return m_map[key]; }

    bool Lookup(const K& key, V& val) const {
        auto it = m_map.find(key);
        if (it != m_map.end()) { val = it->second; return true; }
        return false;
    }

    // MFC-style Lookup returning CPair* (returns null if not found)
    CPair* Lookup(const K& key) {
        auto it = m_map.find(key);
        if (it == m_map.end()) return nullptr;
        // Store in thread-local to return stable pointer
        static thread_local CPair s_pair;
        s_pair.m_key = it->first;
        s_pair.m_value = it->second;
        return &s_pair;
    }

    void SetAt(const K& key, const V& val) { m_map[key] = val; }
    bool RemoveKey(const K& key) { return m_map.erase(key) > 0; }

    // MFC-style iteration using POSITION
    // We encode position as (index + 1) cast to void*
    POSITION GetStartPosition() const {
        if (m_map.empty()) return nullptr;
        return (POSITION)(uintptr_t)1;
    }

    const CPair* GetNext(POSITION& pos) const {
        if (!pos) return nullptr;
        uintptr_t idx = (uintptr_t)pos - 1;
        auto it = m_map.begin();
        std::advance(it, idx);
        static thread_local CPair s_pair;
        s_pair.m_key = it->first;
        s_pair.m_value = it->second;
        ++idx;
        pos = (idx < m_map.size()) ? (POSITION)(uintptr_t)(idx + 1) : nullptr;
        return &s_pair;
    }

    V& GetNextValue(POSITION& pos) {
        uintptr_t idx = (uintptr_t)pos - 1;
        auto it = m_map.begin();
        std::advance(it, idx);
        V& val = it->second;
        ++idx;
        pos = (idx < m_map.size()) ? (POSITION)(uintptr_t)(idx + 1) : nullptr;
        return val;
    }

    const K& GetNextKey(POSITION& pos) const {
        uintptr_t idx = (uintptr_t)pos - 1;
        auto it = m_map.begin();
        std::advance(it, idx);
        const K& key = it->first;
        ++idx;
        pos = (idx < m_map.size()) ? (POSITION)(uintptr_t)(idx + 1) : nullptr;
        return key;
    }

    void GetNextAssoc(POSITION& pos, K& key, V& val) const {
        if (!pos) return;
        uintptr_t idx = (uintptr_t)pos - 1;
        auto it = m_map.begin();
        std::advance(it, idx);
        key = it->first;
        val = it->second;
        ++idx;
        pos = (idx < m_map.size()) ? (POSITION)(uintptr_t)(idx + 1) : nullptr;
    }

    void RemoveAtPos(POSITION pos) {
        uintptr_t idx = (uintptr_t)pos - 1;
        auto it = m_map.begin();
        std::advance(it, idx);
        m_map.erase(it);
    }
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

    // MFC CAutoPtr allows copy (transfers ownership)
    CAutoPtr(CAutoPtr& other) : m_ptr(other.m_ptr) { other.m_ptr = nullptr; }
    CAutoPtr& operator=(CAutoPtr& other) {
        if (this != &other) { delete m_ptr; m_ptr = other.m_ptr; other.m_ptr = nullptr; }
        return *this;
    }
};

template<typename T>
class CAutoVectorPtr {
public:
    T* m_p;
    CAutoVectorPtr() : m_p(nullptr) {}
    ~CAutoVectorPtr() { delete[] m_p; }
    bool Allocate(size_t count) { delete[] m_p; m_p = new(std::nothrow) T[count](); return m_p != nullptr; }
    void Attach(T* p) { delete[] m_p; m_p = p; }
    T* Detach() { T* p = m_p; m_p = nullptr; return p; }
    operator T*() { return m_p; }
    operator const T*() const { return m_p; }
    CAutoVectorPtr(const CAutoVectorPtr&) = delete;
    CAutoVectorPtr& operator=(const CAutoVectorPtr&) = delete;
};

template<typename T>
class CAutoPtrList {
    std::list<CAutoPtr<T>> m_list;

    typename std::list<CAutoPtr<T>>::iterator PosToIter(POSITION pos) {
        for (auto it = m_list.begin(); it != m_list.end(); ++it) {
            if ((POSITION)&(*it) == pos) return it;
        }
        return m_list.end();
    }
    typename std::list<CAutoPtr<T>>::const_iterator PosToIter(POSITION pos) const {
        for (auto it = m_list.begin(); it != m_list.end(); ++it) {
            if ((POSITION)&(*it) == pos) return it;
        }
        return m_list.end();
    }
public:
    size_t GetCount() const { return m_list.size(); }
    bool IsEmpty() const { return m_list.empty(); }
    void RemoveAll() { m_list.clear(); }
    void AddTail(CAutoPtr<T>& p) { m_list.push_back(std::move(p)); }
    void AddHead(CAutoPtr<T>& p) { m_list.push_front(std::move(p)); }

    POSITION GetHeadPosition() const {
        if (m_list.empty()) return nullptr;
        return (POSITION)&m_list.front();
    }
    POSITION GetTailPosition() const {
        if (m_list.empty()) return nullptr;
        return (POSITION)&m_list.back();
    }

    CAutoPtr<T>& GetNext(POSITION& pos) {
        CAutoPtr<T>& val = *(CAutoPtr<T>*)pos;
        auto it = PosToIter(pos);
        ++it;
        pos = (it == m_list.end()) ? nullptr : (POSITION)&(*it);
        return val;
    }
    const CAutoPtr<T>& GetNext(POSITION& pos) const {
        const CAutoPtr<T>& val = *(const CAutoPtr<T>*)pos;
        auto it = PosToIter(pos);
        ++it;
        pos = (it == m_list.end()) ? nullptr : (POSITION)&(*it);
        return val;
    }

    CAutoPtr<T>& GetPrev(POSITION& pos) {
        CAutoPtr<T>& val = *(CAutoPtr<T>*)pos;
        auto it = PosToIter(pos);
        if (it == m_list.begin()) pos = nullptr;
        else { --it; pos = (POSITION)&(*it); }
        return val;
    }

    CAutoPtr<T>& GetAt(POSITION pos) { return *(CAutoPtr<T>*)pos; }
    const CAutoPtr<T>& GetAt(POSITION pos) const { return *(const CAutoPtr<T>*)pos; }

    POSITION InsertBefore(POSITION pos, CAutoPtr<T>& val) {
        auto it = PosToIter(pos);
        auto newit = m_list.insert(it, std::move(val));
        return (POSITION)&(*newit);
    }

    T* GetHead() { return m_list.empty() ? nullptr : m_list.front().operator->(); }
    T* GetTail() { return m_list.empty() ? nullptr : m_list.back().operator->(); }

    CAutoPtr<T> RemoveHead() {
        CAutoPtr<T> val = std::move(m_list.front());
        m_list.pop_front();
        return val;
    }
    CAutoPtr<T> RemoveTail() {
        CAutoPtr<T> val = std::move(m_list.back());
        m_list.pop_back();
        return val;
    }

    void RemoveHeadNoReturn() {
        if (!m_list.empty()) m_list.pop_front();
    }

    void RemoveAt(POSITION pos) {
        auto it = PosToIter(pos);
        if (it != m_list.end()) m_list.erase(it);
    }

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
    CComQIPtr(U* p) : CComPtr<T>(static_cast<T*>(p)) {}
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
// CFile / CStdioFile - MFC file replacement using FILE*
// ============================================================

class CFile {
public:
    enum OpenFlags {
        modeRead      = 0x0001,
        modeWrite     = 0x0002,
        modeCreate    = 0x1000,
        typeBinary    = 0x2000,
        typeText      = 0x4000,
        shareDenyNone = 0x0040,
        shareDenyWrite = 0x0020,
    };
    enum SeekPosition { begin = 0, current = 1, end = 2 };

    CString m_strFileName;

    CFile() : m_pFile(nullptr) {}
    virtual ~CFile() { Close(); }

    virtual bool Open(LPCTSTR lpszFileName, UINT nOpenFlags) {
        Close();
        m_strFileName = lpszFileName;

        const wchar_t* mode = L"rb";
        if ((nOpenFlags & modeCreate) && (nOpenFlags & modeWrite)) {
            mode = (nOpenFlags & typeBinary) ? L"wb" : L"w";
        } else if (nOpenFlags & modeWrite) {
            mode = (nOpenFlags & typeBinary) ? L"r+b" : L"r+";
        } else if (nOpenFlags & modeRead) {
            if (nOpenFlags & typeBinary) mode = L"rb";
            else mode = L"r";
        }

        // Convert wchar_t filename to char for fopen
        const wchar_t* wfn = (const wchar_t*)m_strFileName;
        std::string fn8;
        while (*wfn) { fn8 += (char)(*wfn & 0x7F); wfn++; }
        m_pFile = fopen(fn8.c_str(), fn8.c_str() ? (const char*)CStringA(CStringW(mode)) : "rb");
        return m_pFile != nullptr;
    }

    virtual void Close() {
        if (m_pFile) { fclose(m_pFile); m_pFile = nullptr; }
    }

    UINT Read(void* buf, UINT count) {
        if (!m_pFile) return 0;
        return (UINT)fread(buf, 1, count, m_pFile);
    }

    void Write(const void* buf, UINT count) {
        if (m_pFile) fwrite(buf, 1, count, m_pFile);
    }

    ULONGLONG GetPosition() const {
        if (!m_pFile) return 0;
        return (ULONGLONG)ftell(m_pFile);
    }

    ULONGLONG GetLength() const {
        if (!m_pFile) return 0;
        long cur = ftell(m_pFile);
        fseek(m_pFile, 0, SEEK_END);
        long len = ftell(m_pFile);
        fseek(m_pFile, cur, SEEK_SET);
        return (ULONGLONG)len;
    }

    ULONGLONG Seek(LONGLONG lOff, UINT nFrom) {
        if (!m_pFile) return 0;
        int origin = SEEK_SET;
        if (nFrom == current) origin = SEEK_CUR;
        else if (nFrom == end) origin = SEEK_END;
        fseek(m_pFile, (long)lOff, origin);
        return (ULONGLONG)ftell(m_pFile);
    }

    CString GetFilePath() const { return m_strFileName; }

protected:
    FILE* m_pFile;
};

class CStdioFile : public CFile {
public:
    using CFile::CFile;

    BOOL ReadString(CString& str) {
        if (!m_pFile) return FALSE;
        str.Empty();
        char buf[4096];
        if (!fgets(buf, sizeof(buf), m_pFile)) return FALSE;
        // Remove trailing newline
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[--len] = '\0';
        if (len > 0 && buf[len-1] == '\r') buf[--len] = '\0';
        str = CString(buf);
        return TRUE;
    }

    void WriteString(LPCTSTR lpsz) {
        if (!m_pFile) return;
        CStringA a((const wchar_t*)CString(lpsz));
        fputs((const char*)a, m_pFile);
    }

    void Flush() {
        if (m_pFile) fflush(m_pFile);
    }
};

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
#define interface struct

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
// Use NonDelegatingQueryInterface for QI, simple ref counting for AddRef/Release.
// Avoids ambiguous CUnknown:: base in diamond inheritance.
#define DECLARE_IUNKNOWN \
    HRESULT QueryInterface(REFIID riid, void** ppv) override { return NonDelegatingQueryInterface(riid, ppv); } \
    ULONG AddRef() override { return 1; } \
    ULONG Release() override { return 1; }

// Media type enums - defined in MemSubPic.h, not here
// to avoid conflicts

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

// GetInterface (DirectShow helper)
inline HRESULT GetInterface(IUnknown* pUnk, void** ppv) {
    if (!ppv) return E_POINTER;
    *ppv = pUnk;
    pUnk->AddRef();
    return S_OK;
}

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

// GDI path types
#define PT_CLOSEFIGURE  0x01
#define PT_LINETO       0x02
#define PT_BEZIERTO     0x04
#define PT_MOVETO       0x06

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
