
1.创建 ts, 内容如下： 
```js
import { useEffect, useState } from 'react'
export function useWindowSize() {
  const [size, setSize] = useState({
    width: document.documentElement.clientWidth,
    height: document.documentElement.clientHeight
  })

  const handleResize = () => {
    setSize({
      width: document.documentElement.clientWidth,
      height: document.documentElement.clientHeight
    })
  }

  // 实时获取 浏览器 宽高
  useEffect(() => {
    window.addEventListener('resize', handleResize)
    return () => {
      window.removeEventListener('resize', handleResize)
    }
  }, [])
  return [size]
}

```

2.导入 funtion
```js
import { useWindowSize } from './useWindowSize'
```

3.调用
```js
const [size] = useWindowSize()
```