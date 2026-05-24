import practice_module
practice_module.price(3)
practice_module.price_morning(6)
practice_module.price_soldier(15)

import practice_module as mv # as => 별명을 붙이겠다(이름이 길 때)
mv.price(3)

from practice_module import * # 전부 사용 => 모듈명 적을 필요 없음
price(3)
price_morning(9)

from practice_module import price, price_morning
# => price_soldier은 쓸 수 없음

패키지