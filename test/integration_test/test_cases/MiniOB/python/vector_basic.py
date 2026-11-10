import __init__
from test_case import TestCase

def create_test_cases() -> TestCase:
  case = TestCase()
  case.name = "vector_basic"
  case.need_mysql = False

  # 正常流程：建表、插入、查询
  g = case.add_execution_group('vector basic')
  g.add_sql_instruction('create table t (id int, c1 vector(3));', expected='SUCCESS')
  g.add_sql_instruction("insert into t values(1, STRING_TO_VECTOR('[1,2,3]'));", expected='SUCCESS')
  g.add_sql_instruction("insert into t values(2, TO_VECTOR('[4,5,6]'));", expected='SUCCESS')
  g.add_sql_instruction("insert into t values(3, string_to_vector('[1e-3,-7.42E2,.5]'));", expected='SUCCESS')
  g.add_sql_instruction("insert into t values(4, string_to_vector('  [ 7, 8 , 9 ]  '));", expected='SUCCESS')
  g.add_sql_instruction('select * from t;',
                        expected_messages=['id | c1', '1 | [1,2,3]', '2 | [4,5,6]',
                                           '3 | [0.001,-742,0.5]', '4 | [7,8,9]'])
  g.add_sql_instruction('select id, VECTOR_TO_STRING(c1) from t where id=1;',
                        expected_messages=['id | VECTOR_TO_STRING(c1)', '1 | [1,2,3]'])
  g.add_sql_instruction('select id, FROM_VECTOR(c1) from t where id=2;',
                        expected_messages=['id | FROM_VECTOR(c1)', '2 | [4,5,6]'])
  g.add_sql_instruction("select id from t where c1 = STRING_TO_VECTOR('[1,2,3]');",
                        expected_messages=['id', '1'])
  g.add_sql_instruction("select id from t where c1 <> STRING_TO_VECTOR('[1,2,3]');",
                        expected_messages=['id', '2', '3', '4'])

  # 无 FROM 的 CALC 函数表达式
  cg = case.add_execution_group('calc')
  cg.add_sql_instruction("calc VECTOR_TO_STRING(STRING_TO_VECTOR('[1.05,-17.8,32]'));",
                         expected_messages=["VECTOR_TO_STRING(STRING_TO_VECTOR('[1.05,-17.8,32]'))",
                                            '[1.05,-17.8,32]'])

  # 报错场景
  eg = case.add_execution_group('error')
  eg.add_sql_instruction('select id from t where c1 = 1;', expected='FAILURE')
  eg.add_sql_instruction("select id from t where c1 = 'abc';", expected='FAILURE')
  eg.add_sql_instruction("select id from t where c1 < STRING_TO_VECTOR('[1,2,3]');", expected='FAILURE')
  eg.add_sql_instruction('create table bad1 (c vector());', expected='FAILURE')
  eg.add_sql_instruction('create table bad2 (c vector(-1));', expected='FAILURE')
  eg.add_sql_instruction('create table bad3 (c vector(0));', expected='FAILURE')
  eg.add_sql_instruction('create table bad4 (c vector(16384));', expected='FAILURE')
  eg.add_sql_instruction("insert into t values(9, string_to_vector('[1,2,3,]'));", expected='FAILURE')
  eg.add_sql_instruction("insert into t values(10, string_to_vector('[1,2,3,4]'));", expected='FAILURE')
  eg.add_sql_instruction("insert into t values(11, string_to_vector('abc'));", expected='FAILURE')

  return case
